#!/bin/bash
# ============================================================================
# Webserv — black-box HTTP test runner
# ============================================================================
# Self-contained: generates its own webroot + config, starts the server,
# runs the HTTP-* / MTH-* / STATUS-* / CONN-* / ROBUST-* cases from
# test/TESTING_PLAN.md, and reports PASS/FAIL. Exit code = number of failures.
#
# Usage:   ./test/http_tests/run.sh [port]
# Env:     WEBSERV_BIN (default ./webserv)   PORT (default 8090)
#
# Tests tagged [KNOWN-GAP] target a bug CONFIRMED by code reading
# (see TESTING_PLAN.md §5). A FAIL there is expected on the current code and
# is diagnostic, not a runner error — it flips to PASS once the bug is fixed.
# ============================================================================

set -u

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
WEBROOT="$SCRIPT_DIR/www"
CONF="$SCRIPT_DIR/http_test.conf"
LOGDIR="$SCRIPT_DIR/logs"
SRVLOG="$LOGDIR/server.out"
PORT="${1:-${PORT:-8090}}"
HOST="127.0.0.1"
BASE="http://$HOST:$PORT"
# Second server for CGI/DAV tests: a MINIMAL config (/, /cgi-bin, /webdav).
# The main config's rich location set triggers a config-sensitivity bug (F7)
# that spuriously breaks CGI/body handling, so those features are exercised
# here on a clean config where they are known to work.
PORT2=$((PORT + 1))
CONF2="$SCRIPT_DIR/http_test_cgi.conf"
BASE2="http://$HOST:$PORT2"
WEBSERV_BIN="${WEBSERV_BIN:-$REPO_DIR/webserv}"

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[0;33m'; DIM='\033[2m'; NC='\033[0m'
PASS=0; FAIL=0; TOTAL=0; SRV_PID=""; SRV2_PID=""

# ---------------------------------------------------------------- infra
cleanup() {
	for p in "$SRV_PID" "$SRV2_PID"; do
		if [ -n "$p" ] && kill -0 "$p" 2>/dev/null; then kill "$p" 2>/dev/null; wait "$p" 2>/dev/null; fi
	done
}
trap cleanup EXIT INT TERM

setup_webroot() {
	rm -rf "$WEBROOT"; mkdir -p "$WEBROOT/browse" "$WEBROOT/getonly" "$WEBROOT/webdav" "$WEBROOT/cgi-bin" "$LOGDIR"
	printf 'WEBSERV_TEST_INDEX\n' > "$WEBROOT/index.html"
	printf 'hello webserv\n'      > "$WEBROOT/hello.txt"
	printf 'CUSTOM_404_PAGE\n'    > "$WEBROOT/404.html"
	printf 'browse-note\n'        > "$WEBROOT/browse/note.txt"
	printf 'GETONLY_INDEX\n'      > "$WEBROOT/getonly/index.html"
	# CGI scripts — MUST be executable (webserv returns 403 otherwise)
	printf '#!/usr/bin/env python3\nprint("Content-Type: text/plain")\nprint()\nprint("CGI_HELLO")\n' > "$WEBROOT/cgi-bin/hello.py"
	printf '#!/usr/bin/env python3\nimport os,sys\nn=int(os.environ.get("CONTENT_LENGTH") or 0)\nb=sys.stdin.read(n) if n else ""\nprint("Content-Type: text/plain");print()\nprint("METHOD="+str(os.environ.get("REQUEST_METHOD")))\nprint("QUERY="+str(os.environ.get("QUERY_STRING")))\nprint("CLEN="+str(os.environ.get("CONTENT_LENGTH")))\nprint("BODY="+b)\n' > "$WEBROOT/cgi-bin/info.py"
	printf '#!/usr/bin/env python3\nprint("Status: 302 Found")\nprint("Location: /index.html")\nprint()\n' > "$WEBROOT/cgi-bin/status.py"
	printf '#!/usr/bin/env python3\nimport sys\nsys.exit(1)\n' > "$WEBROOT/cgi-bin/crash.py"
	chmod +x "$WEBROOT/cgi-bin/hello.py" "$WEBROOT/cgi-bin/info.py" "$WEBROOT/cgi-bin/status.py" "$WEBROOT/cgi-bin/crash.py"
	CGI_PY="$(command -v python3 || echo /usr/bin/python3)"
}

gen_config() {
	# Structure modeled on example_configs/webserv.conf so it matches the
	# strict ABNF grammar (types / cgi_extensions / dav_* blocks are required;
	# `index` is only valid inside a location; last cgi entry has no ';').
	cat > "$CONF" <<EOF
http {
	types {
		text/html	html htm;
		text/css	css;
		text/plain	txt;
		image/png	png;
	}
	client_max_body_size 10m;
	default_type text/plain;
	error_page 404 /404.html;
	keepalive_timeout 65;
	root $WEBROOT;
	enable_cgi on;
	cgi_extensions {
		/usr/bin/python3	.py;
		/bin/bash		.sh
	}
	create_full_put_path on;
	dav_access user:rw;
	dav_methods off;
	dav_put_path: /;

	server {
		client_max_body_size 10m;
		default_type text/plain;
		error_page 404 /404.html;
		keepalive_timeout 65;
		listen $HOST:$PORT default_server;
		root $WEBROOT;
		server_name localhost;

		types {
			text/html	html htm;
			text/css	css;
		}
		enable_cgi on;
		cgi_extensions {
			/usr/bin/python3	.py;
			/bin/bash		.sh
		}
		create_full_put_path on;
		dav_access user:rw;
		dav_methods off;
		dav_put_path: /;

		location / {
			autoindex off;
			index index.html;
			allowed_methods GET HEAD POST;
			root $WEBROOT;
		}
		location /browse {
			autoindex on;
			allowed_methods GET;
			root $WEBROOT;
		}
		location /getonly {
			index index.html;
			allowed_methods GET;
			root $WEBROOT;
		}
		location /small {
			client_max_body_size 100;
			allowed_methods GET POST PUT;
			create_full_put_path on;
			dav_access user:rw;
			dav_methods PUT;
			dav_put_path: /uploads;
			root $WEBROOT;
		}
		location = /old {
			return 301 http://example.com/new;
		}
		location = /temp {
			return 302 /index.html;
		}
	}
}
EOF
}

start_server() {
	if [ ! -x "$WEBSERV_BIN" ]; then
		echo -e "${YELLOW}Binary $WEBSERV_BIN not found — building...${NC}"
		make -C "$REPO_DIR" >/dev/null 2>&1 || { echo -e "${RED}Build failed${NC}"; exit 2; }
	fi
	( cd "$REPO_DIR" && "$WEBSERV_BIN" "$CONF" ) >"$SRVLOG" 2>&1 &
	SRV_PID=$!
	# wait until the port answers (max ~12s; ABNF grammar load takes ~2-3s)
	local i=0
	while [ $i -lt 120 ]; do
		if curl -s -o /dev/null --max-time 1 "$BASE/" 2>/dev/null; then return 0; fi
		if ! kill -0 "$SRV_PID" 2>/dev/null; then
			echo -e "${RED}Server died on startup. Log:${NC}"; tail -20 "$SRVLOG"; exit 2
		fi
		sleep 0.1; i=$((i + 1))
	done
	echo -e "${RED}Server did not become ready on $BASE${NC}"; tail -20 "$SRVLOG"; exit 2
}

# Minimal CGI/DAV config — kept small on purpose (see F7 config-sensitivity).
gen_config_cgi() {
	cat > "$CONF2" <<EOF
http {
	types { text/html html htm; text/plain txt; }
	client_max_body_size 10m; default_type text/plain; error_page 404 /404.html;
	keepalive_timeout 65; root $WEBROOT;
	enable_cgi on; cgi_extensions { /usr/bin/python3 .py; /bin/bash .sh; }
	create_full_put_path on; dav_access user:rw; dav_methods off; dav_put_path: /;
	server {
		client_max_body_size 10m; default_type text/plain; error_page 404 /404.html;
		keepalive_timeout 65; listen $HOST:$PORT2 default_server; root $WEBROOT; server_name localhost;
		types { text/html html htm; text/plain txt; }
		enable_cgi on; cgi_extensions { /usr/bin/python3 .py; /bin/bash .sh; }
		create_full_put_path on; dav_access user:rw; dav_methods off; dav_put_path: /;
		location / { autoindex off; index index.html; allowed_methods GET HEAD POST; root $WEBROOT; }
		location /cgi-bin { allowed_methods GET POST; enable_cgi on; client_max_body_size 10M; root $WEBROOT; }
		location /webdav { autoindex on; allowed_methods GET HEAD PUT DELETE; create_full_put_path on; dav_methods PUT DELETE; dav_access user:rwx; dav_put_path: $WEBROOT; root $WEBROOT; }
	}
}
EOF
	( cd "$REPO_DIR" && "$WEBSERV_BIN" "$CONF2" ) >"$LOGDIR/server2.out" 2>&1 &
	SRV2_PID=$!
	local i=0
	while [ $i -lt 120 ]; do
		curl -s -o /dev/null --max-time 1 "$BASE2/" 2>/dev/null && return 0
		kill -0 "$SRV2_PID" 2>/dev/null || { echo -e "${RED}CGI server died${NC}"; tail -10 "$LOGDIR/server2.out"; return 1; }
		sleep 0.1; i=$((i + 1))
	done
	echo -e "${RED}CGI server not ready on $BASE2${NC}"; return 1
}
stop_server2() { [ -n "$SRV2_PID" ] && kill "$SRV2_PID" 2>/dev/null; wait "$SRV2_PID" 2>/dev/null; SRV2_PID=""; }

# ---------------------------------------------------------------- assertions
# curl status only; --max-time guards against hangs (never let a test block)
_status() { curl -s -o /dev/null -w '%{http_code}' --max-time 5 "$@" 2>/dev/null; }

pass() { echo -e "${GREEN}[PASS]${NC} $1"; PASS=$((PASS + 1)); }
fail() { echo -e "${RED}[FAIL]${NC} $1 ${DIM}-> $2${NC}"; FAIL=$((FAIL + 1)); }

# expect_status ID "desc" EXPECTED curl-args...
expect_status() {
	local id="$1" desc="$2" exp="$3"; shift 3
	TOTAL=$((TOTAL + 1))
	local got; got=$(_status "$@")
	if [ "$got" = "$exp" ]; then pass "$id $desc"; else fail "$id $desc" "expected $exp, got ${got:-<none/timeout>}"; fi
}

# expect_header ID "desc" HEADER_REGEX curl-args...
expect_header() {
	local id="$1" desc="$2" rx="$3"; shift 3
	TOTAL=$((TOTAL + 1))
	local hdrs; hdrs=$(curl -s -D - -o /dev/null --max-time 5 "$@" 2>/dev/null)
	if echo "$hdrs" | grep -iqE "$rx"; then pass "$id $desc"; else fail "$id $desc" "no header matching /$rx/"; fi
}

# expect_body ID "desc" SUBSTRING curl-args...
expect_body() {
	local id="$1" desc="$2" sub="$3"; shift 3
	TOTAL=$((TOTAL + 1))
	local body; body=$(curl -s --max-time 5 "$@" 2>/dev/null)
	if echo "$body" | grep -qF "$sub"; then pass "$id $desc"; else fail "$id $desc" "body missing '$sub'"; fi
}

# raw_expect ID "desc" STATUS_REGEX  <python-sends-bytes-and-prints-response>
# Sends raw bytes via python socket (deterministic framing) and matches the
# first response line. Used for tests curl cannot express.
raw_expect() {
	local id="$1" desc="$2" rx="$3" py="$4"
	TOTAL=$((TOTAL + 1))
	local resp; resp=$(python3 -c "$py" 2>/dev/null)
	if echo "$resp" | grep -qE "$rx"; then pass "$id $desc"; else fail "$id $desc" "status line !~ /$rx/ (got: $(echo "$resp" | head -1))"; fi
}

# assert_alive: after a hostile test, the server MUST still serve (never-crash)
assert_alive() {
	local id="$1"
	TOTAL=$((TOTAL + 1))
	if kill -0 "$SRV_PID" 2>/dev/null && [ "$(_status "$BASE/")" = "200" ]; then
		pass "$id server still alive after hostile input"
	else
		fail "$id server DEAD or unresponsive" "CRITICAL — grade-0 (REQ-11)"
	fi
}

py_send() {
	# emits a python one-liner: connect, send $1 (raw, python-escaped), print reply
	printf "import socket;s=socket.create_connection(('%s',%s),timeout=4);s.sendall(%s);import sys;d=b''\nwhile True:\n try:\n  c=s.recv(4096)\n except: break\n if not c: break\n d+=c\nsys.stdout.write(d.decode('latin1'))" "$HOST" "$PORT" "$1"
}

# ---------------------------------------------------------------- run
echo "=============================================="
echo " Webserv HTTP black-box tests  ($BASE)"
echo "=============================================="
setup_webroot
gen_config
start_server
echo -e "${DIM}server pid=$SRV_PID  conf=$CONF${NC}\n"

# ── Static serving & GET ─────────────────────────────────────────────────
echo -e "${YELLOW}--- Static & GET ---${NC}"
expect_status GET-01 "GET /index.html -> 200"          200 "$BASE/index.html"
expect_body   GET-02 "GET / serves index body"         "WEBSERV_TEST_INDEX" "$BASE/"
expect_status GET-03 "GET /hello.txt -> 200"           200 "$BASE/hello.txt"
expect_status GET-04 "GET /nope -> 404"                404 "$BASE/nope"
expect_body   GET-05 "404 serves custom error page"    "CUSTOM_404_PAGE" "$BASE/nope"
expect_header GET-06 "response has Content-Length"      "^Content-Length:" "$BASE/index.html"
expect_status GET-07 "autoindex dir -> 200"            200 "$BASE/browse/"
expect_body   GET-08 "autoindex lists note.txt"        "note.txt" "$BASE/browse/"

# ── Methods ──────────────────────────────────────────────────────────────
echo -e "\n${YELLOW}--- Methods ---${NC}"
expect_status MTH-01 "POST /getonly -> 405"            405 -X POST "$BASE/getonly/"
expect_header MTH-02 "405 carries Allow header"         "^Allow:" -X POST "$BASE/getonly/"
expect_status MTH-03 "DELETE /getonly -> 405"          405 -X DELETE "$BASE/getonly/"

# ── Redirects / status codes ─────────────────────────────────────────────
echo -e "\n${YELLOW}--- Redirects & status ---${NC}"
expect_status ST-01 "return 301"                       301 "$BASE/old"
expect_header ST-02 "301 Location header"               "^Location: ?http://example.com/new" "$BASE/old"
expect_status ST-03 "return 302"                       302 "$BASE/temp"

# ── Body size limit ──────────────────────────────────────────────────────
echo -e "\n${YELLOW}--- Body limits ---${NC}"
# BODY-01 [KNOWN-GAP]: the server currently answers 415 to ANY body-bearing
# request (POST/PUT, any size, any route), so the 413 size-limit path is
# unreachable in black-box tests. Expect 413; a 415 flags that body handling
# rejects the request before the size check (see report findings F2).
expect_status BODY-01 "[KNOWN-GAP] POST 200B > 100 limit -> 413" 413 -X POST --data "$(head -c 200 </dev/zero | tr '\0' 'A')" "$BASE/small/x"

# ── Framing (raw bytes) ──────────────────────────────────────────────────
echo -e "\n${YELLOW}--- Framing (raw) ---${NC}"
# HTTP-06: missing Host on HTTP/1.1 must be 400 (RFC 9112 §3.2)
raw_expect HTTP-06 "missing Host/1.1 -> 400" "^HTTP/1\.[01] 400" \
	"$(py_send "b'GET / HTTP/1.1\r\n\r\n'")"
# HTTP-10: non-digit Content-Length -> 400
raw_expect HTTP-10 "bad Content-Length -> 400" "^HTTP/1\.[01] 400" \
	"$(py_send "b'POST /small/x HTTP/1.1\r\nHost: x\r\nContent-Length: abc\r\n\r\n'")"
# HTTP-11 [KNOWN-GAP §5 R/smuggling]: CL + TE both present -> must be 400
raw_expect HTTP-11 "[KNOWN-GAP] CL+TE both -> 400" "^HTTP/1\.[01] 400" \
	"$(py_send "b'POST /small/x HTTP/1.1\r\nHost: x\r\nContent-Length: 5\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n'")"
# HTTP-01 [KNOWN-GAP §5 R: segmented headers -> spurious 400]
TOTAL=$((TOTAL + 1))
seg=$(python3 -c "
import socket,time
s=socket.create_connection(('$HOST',$PORT),timeout=4)
s.sendall(b'GET / HTTP/1.1\r\n'); time.sleep(0.7); s.sendall(b'Host: x\r\n\r\n')
print(s.recv(256).decode('latin1').splitlines()[0] if True else '')
" 2>/dev/null)
if echo "$seg" | grep -qE "^HTTP/1\.[01] (200|404)"; then
	pass "HTTP-01 segmented headers buffered (got: $seg)"
else
	fail "HTTP-01 [KNOWN-GAP] segmented headers" "expected 200/404, got '${seg:-<none>}' — confirms framing bug (RequestHandler.cpp:108)"
fi

# ── Robustness / never-crash ─────────────────────────────────────────────
echo -e "\n${YELLOW}--- Robustness ---${NC}"
python3 -c "
import socket
s=socket.create_connection(('$HOST',$PORT),timeout=2)
s.sendall(bytes(range(256))*8)   # 2KB of binary garbage
try: s.recv(256)
except: pass
s.close()" 2>/dev/null
assert_alive ROBUST-01
# oversized request line -> 414 (or at least no crash)
raw_expect ROBUST-02 "huge URI -> 414/400" "^HTTP/1\.[01] (414|400)" \
	"$(py_send "b'GET /' + b'a'*9000 + b' HTTP/1.1\r\nHost: x\r\n\r\n'")"
assert_alive ROBUST-03

# ── Keep-alive ───────────────────────────────────────────────────────────
echo -e "\n${YELLOW}--- Connection ---${NC}"
TOTAL=$((TOTAL + 1))
ka=$(python3 -c "
import socket, re
def read_full(s):
    data=b''
    while b'\r\n\r\n' not in data: data+=s.recv(4096)
    head,_,rest=data.partition(b'\r\n\r\n')
    m=re.search(rb'Content-Length:\s*(\d+)', head)
    n=int(m.group(1)) if m else 0
    while len(rest)<n: rest+=s.recv(4096)
    return rest[:n]
s=socket.create_connection(('$HOST',$PORT),timeout=4)
s.sendall(b'GET /hello.txt HTTP/1.1\r\nHost: x\r\nConnection: keep-alive\r\n\r\n')
b1=read_full(s)
s.sendall(b'GET /index.html HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n')
b2=read_full(s)
print('OK' if (b'hello webserv' in b1 and b'WEBSERV_TEST_INDEX' in b2) else 'BAD')
" 2>/dev/null)
if [ "$ka" = "OK" ]; then pass "CONN-01 two requests on one keep-alive connection"; else fail "CONN-01 keep-alive" "second request not served (got: ${ka:-<none>})"; fi

# ── Methods: HEAD (main server) ──────────────────────────────────────────
echo -e "\n${YELLOW}--- Methods (HEAD) ---${NC}"
expect_status MTH-04 "HEAD /index.html -> 200"          200 -I "$BASE/index.html"
expect_header MTH-05 "HEAD has Content-Length"           "^Content-Length:" -I "$BASE/index.html"

# ── CGI / DAV / chunked (on the clean 2nd server, port $PORT2) ────────────
echo -e "\n${YELLOW}--- CGI / DAV / chunked (clean config, :$PORT2) ---${NC}"
if gen_config_cgi; then
	rm -f "$WEBROOT/webdav/up.txt"
	expect_status DAV-01 "PUT upload (text/plain) -> 201"   201 -X PUT -H "Content-Type: text/plain" --data "UPLOADED_OK" "$BASE2/webdav/up.txt"
	expect_body   DAV-02 "GET uploaded file -> content"     "UPLOADED_OK" "$BASE2/webdav/up.txt"
	expect_status DAV-03 "DELETE uploaded file -> 204"      204 -X DELETE "$BASE2/webdav/up.txt"
	expect_status DAV-04 "GET after DELETE -> 404"          404 "$BASE2/webdav/up.txt"

	expect_status CGI-01 "GET executable CGI -> 200"        200 "$BASE2/cgi-bin/hello.py"
	expect_body   CGI-02 "CGI stdout returned"              "CGI_HELLO" "$BASE2/cgi-bin/hello.py"
	expect_body   CGI-03 "CGI QUERY_STRING passed"          "QUERY=a=42" "$BASE2/cgi-bin/info.py?a=42"
	expect_body   CGI-04 "CGI POST body -> stdin+CLEN"      "BODY=hello-body" -X POST -H "Content-Type: text/plain" --data "hello-body" "$BASE2/cgi-bin/info.py"
	expect_status CGI-05 "CGI crash (exit 1) -> 502"        502 "$BASE2/cgi-bin/crash.py"
	cp "$WEBROOT/cgi-bin/hello.py" "$WEBROOT/cgi-bin/noexec.py"; chmod -x "$WEBROOT/cgi-bin/noexec.py"
	expect_status CGI-06 "non-executable CGI -> 403"        403 "$BASE2/cgi-bin/noexec.py"
	expect_status CGI-07 "[KNOWN-GAP] CGI Status: 302 honored" 302 "$BASE2/cgi-bin/status.py"

	expect_status CT-01 "text/plain body accepted -> 200"   200 -X POST -H "Content-Type: text/plain" --data "x" "$BASE2/cgi-bin/info.py"
	expect_status CT-02 "[KNOWN-GAP] urlencoded form POST -> 200" 200 -X POST --data "x=1" "$BASE2/cgi-bin/info.py"
	expect_status CT-03 "[KNOWN-GAP] application/json body -> 200" 200 -X POST -H "Content-Type: application/json" --data '{}' "$BASE2/cgi-bin/info.py"

	TOTAL=$((TOTAL + 1))
	ch=$(python3 -c "
import socket,time
s=socket.create_connection(('$HOST',$PORT2),timeout=6)
s.sendall(b'POST /cgi-bin/info.py HTTP/1.1\r\nHost: x\r\nContent-Type: text/plain\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n')
time.sleep(0.6); print(s.recv(8192).decode('latin1'))
" 2>/dev/null)
	if echo "$ch" | grep -q "BODY=hello world" && echo "$ch" | grep -q "CLEN=11"; then
		pass "CHUNK-01 chunked request de-chunked for CGI (CLEN=11, body ok)"
	else
		fail "CHUNK-01 chunked de-chunking" "CGI did not see de-chunked body/CONTENT_LENGTH"
	fi
	stop_server2
else
	echo -e "${YELLOW}(skipped CGI/DAV — 2nd server failed to start)${NC}"
fi

# ---------------------------------------------------------------- summary
echo ""
echo "=============================================="
echo -e " Results: ${GREEN}$PASS passed${NC}, ${RED}$FAIL failed${NC}, $TOTAL total"
echo "=============================================="
echo -e "${DIM}[KNOWN-GAP] failures are expected on current code (see TESTING_PLAN.md §5).${NC}"
exit $FAIL
