#!/bin/bash

# Parser config tests
# Usage: ./test/parser_tests/run_tests.sh

WEBSERV="./webserv"
CONFIGS="test/parser_tests/configs"
LOGFILE="logs/webserv/parser.log"
PASS=0
FAIL=0
TOTAL=0

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

assert_log() {
	local test_name="$1"
	local config="$2"
	local pattern="$3"
	local expected="$4"
	TOTAL=$((TOTAL + 1))

	> "$LOGFILE" 2>/dev/null
	$WEBSERV "$config" >/dev/null 2>&1
	actual=$(grep -m1 "$pattern" "$LOGFILE" 2>/dev/null)

	if echo "$actual" | grep -q "$expected"; then
		echo -e "${GREEN}[PASS]${NC} $test_name"
		PASS=$((PASS + 1))
	else
		echo -e "${RED}[FAIL]${NC} $test_name"
		echo "       pattern:  '$pattern'"
		echo "       expected: '$expected'"
		echo "       got:      '$actual'"
		FAIL=$((FAIL + 1))
	fi
}

assert_log_count() {
	local test_name="$1"
	local config="$2"
	local pattern="$3"
	local expected_count="$4"
	TOTAL=$((TOTAL + 1))

	> "$LOGFILE" 2>/dev/null
	$WEBSERV "$config" >/dev/null 2>&1
	actual_count=$(grep -c "$pattern" "$LOGFILE" 2>/dev/null)

	if [ "$actual_count" -eq "$expected_count" ]; then
		echo -e "${GREEN}[PASS]${NC} $test_name"
		PASS=$((PASS + 1))
	else
		echo -e "${RED}[FAIL]${NC} $test_name"
		echo "       pattern:        '$pattern'"
		echo "       expected count: $expected_count"
		echo "       actual count:   $actual_count"
		FAIL=$((FAIL + 1))
	fi
}

assert_fail() {
	local test_name="$1"
	local config="$2"
	TOTAL=$((TOTAL + 1))

	$WEBSERV "$config" >/dev/null 2>&1
	exit_code=$?

	if [ $exit_code -ne 0 ]; then
		echo -e "${GREEN}[PASS]${NC} $test_name (exit code: $exit_code)"
		PASS=$((PASS + 1))
	else
		echo -e "${RED}[FAIL]${NC} $test_name (expected failure, got exit 0)"
		FAIL=$((FAIL + 1))
	fi
}

echo ""
echo "=============================="
echo " Parser Config Tests"
echo "=============================="
echo ""

# ── full.conf ──────────────────────────────────────────────────────────────

echo -e "${YELLOW}--- full.conf ---${NC}"
assert_log "client_max_body_size 1k" \
	"$CONFIGS/full.conf" "Client max body size" "1024 bytes"
assert_log "create_full_put_path on" \
	"$CONFIGS/full.conf" "Create full PUT path" "on"
assert_log "dav_put_path /uploads" \
	"$CONFIGS/full.conf" "DAV PUT path" "/uploads"
assert_log "dav_access user:rw group:rw all:r" \
	"$CONFIGS/full.conf" "DAV access" "0664"
assert_log "dav_methods PUT DELETE" \
	"$CONFIGS/full.conf" "DAV methods" "PUT DELETE"
assert_log "default_type application/octet-stream" \
	"$CONFIGS/full.conf" "Default type" "application/octet-stream"
assert_log "keepalive_timeout 120" \
	"$CONFIGS/full.conf" "Keepalive timeout" "120s"
assert_log "root /var/www/html" \
	"$CONFIGS/full.conf" "Root:" "/var/www/html"
assert_log "enable_cgi on" \
	"$CONFIGS/full.conf" "Enable CGI" "on"
assert_log "types html=text/html" \
	"$CONFIGS/full.conf" "Types:" "html=text/html"
assert_log "types mp4=video/mp4" \
	"$CONFIGS/full.conf" "Types:" "mp4=video/mp4"
assert_log "cgi .py=/usr/bin/python3" \
	"$CONFIGS/full.conf" "CGI extensions:" ".py=/usr/bin/python3"
assert_log "cgi .sh=/usr/bin/bash" \
	"$CONFIGS/full.conf" "CGI extensions:" ".sh=/usr/bin/bash"
assert_log "cgi .php=/usr/bin/php" \
	"$CONFIGS/full.conf" "CGI extensions:" ".php=/usr/bin/php"

# ── error page 50x resolution ─────────────────────────────────────────────

echo ""
echo -e "${YELLOW}--- error page resolution (50x) ---${NC}"
assert_log "error_page 404 -> /404.html" \
	"$CONFIGS/full.conf" "Error page: 404" "/404.html"
assert_log "error_page 500 -> /500.html" \
	"$CONFIGS/full.conf" "Error page: 500" "/500.html"
assert_log "error_page 502 -> /502.html" \
	"$CONFIGS/full.conf" "Error page: 502" "/502.html"
assert_log "error_page 503 -> /503.html" \
	"$CONFIGS/full.conf" "Error page: 503" "/503.html"
assert_log "error_page 504 -> /504.html" \
	"$CONFIGS/full.conf" "Error page: 504" "/504.html"
assert_log_count "error_page total count = 5" \
	"$CONFIGS/full.conf" "Error page:" 5

# ── error page 5xx resolution ─────────────────────────────────────────────

echo ""
echo -e "${YELLOW}--- error page resolution (5xx) ---${NC}"
assert_log "5xx: 500 -> /500.html" \
	"$CONFIGS/error_page_5xx.conf" "Error page: 500" "/500.html"
assert_log "5xx: 501 -> /501.html" \
	"$CONFIGS/error_page_5xx.conf" "Error page: 501" "/501.html"
assert_log "5xx: 502 -> /502.html" \
	"$CONFIGS/error_page_5xx.conf" "Error page: 502" "/502.html"
assert_log "5xx: 503 -> /503.html" \
	"$CONFIGS/error_page_5xx.conf" "Error page: 503" "/503.html"

# ── error page xxx resolution ─────────────────────────────────────────────

echo ""
echo -e "${YELLOW}--- error page resolution (xxx) ---${NC}"
assert_log "xxx: 404 -> /404.html" \
	"$CONFIGS/error_page_xxx.conf" "Error page: 404" "/404.html"
assert_log "xxx: 500 -> /500.html" \
	"$CONFIGS/error_page_xxx.conf" "Error page: 500" "/500.html"
assert_log "xxx: 503 -> /503.html" \
	"$CONFIGS/error_page_xxx.conf" "Error page: 503" "/503.html"

# ── minimal.conf (defaults) ───────────────────────────────────────────────

echo ""
echo -e "${YELLOW}--- minimal.conf (defaults from DefaultConfig) ---${NC}"
assert_log "default client_max_body_size = 1048576" \
	"$CONFIGS/minimal.conf" "Client max body size" "1048576 bytes"
assert_log "default create_full_put_path = off" \
	"$CONFIGS/minimal.conf" "Create full PUT path" "off"
assert_log "default default_type = text/plain" \
	"$CONFIGS/minimal.conf" "Default type" "text/plain"
assert_log "default keepalive_timeout = 75" \
	"$CONFIGS/minimal.conf" "Keepalive timeout" "75s"
assert_log "default root = html" \
	"$CONFIGS/minimal.conf" "Root:" "html"
assert_log "default enable_cgi = off" \
	"$CONFIGS/minimal.conf" "Enable CGI" "off"
assert_log "default dav_access = 0600" \
	"$CONFIGS/minimal.conf" "DAV access" "0600"

# ── size variants ─────────────────────────────────────────────────────────

echo ""
echo -e "${YELLOW}--- size unit variants ---${NC}"
assert_log "size plain bytes 4096" \
	"$CONFIGS/size_bytes.conf" "Client max body size" "4096 bytes"
assert_log "size 10M = 10485760" \
	"$CONFIGS/size_mega.conf" "Client max body size" "10485760 bytes"
assert_log "size 2G = 2147483648" \
	"$CONFIGS/size_giga.conf" "Client max body size" "2147483648 bytes"

# ── cgi directive only ─────────────────────────────────────────────────────

echo ""
echo -e "${YELLOW}--- cgi_extension directives only ---${NC}"
assert_log "cgi_extension .py" \
	"$CONFIGS/cgi_only_dir.conf" "CGI extensions:" ".py=/usr/bin/python3"
assert_log "cgi_extension .sh" \
	"$CONFIGS/cgi_only_dir.conf" "CGI extensions:" ".sh=/usr/bin/bash"

# ── cgi block only ─────────────────────────────────────────────────────────

echo ""
echo -e "${YELLOW}--- cgi_extensions block only ---${NC}"
assert_log "cgi_extensions block .rb" \
	"$CONFIGS/cgi_only_block.conf" "CGI extensions:" ".rb=/usr/bin/ruby"
assert_log "cgi_extensions block .pl" \
	"$CONFIGS/cgi_only_block.conf" "CGI extensions:" ".pl=/usr/bin/perl"

# ── dav_methods off ────────────────────────────────────────────────────────

echo ""
echo -e "${YELLOW}--- dav_methods off ---${NC}"
assert_log "dav_methods off = empty" \
	"$CONFIGS/dav_off.conf" "DAV methods:" "DAV methods: "
assert_log "dav_access user:rwx = 0700" \
	"$CONFIGS/dav_off.conf" "DAV access" "0700"

# ── types only ─────────────────────────────────────────────────────────────

echo ""
echo -e "${YELLOW}--- types block ---${NC}"
assert_log "types html=text/html" \
	"$CONFIGS/types_only.conf" "Types:" "html=text/html"
assert_log "types htm=text/html" \
	"$CONFIGS/types_only.conf" "Types:" "htm=text/html"
assert_log "types jpg=image/jpeg" \
	"$CONFIGS/types_only.conf" "Types:" "jpg=image/jpeg"
assert_log "types jpeg=image/jpeg" \
	"$CONFIGS/types_only.conf" "Types:" "jpeg=image/jpeg"

# ── comments ───────────────────────────────────────────────────────────────

echo ""
echo -e "${YELLOW}--- comments ---${NC}"
assert_log "comment: client_max_body_size 512K" \
	"$CONFIGS/comment.conf" "Client max body size" "524288 bytes"
assert_log "comment: root /var/www" \
	"$CONFIGS/comment.conf" "Root:" "/var/www"
assert_log "comment: keepalive_timeout 30" \
	"$CONFIGS/comment.conf" "Keepalive timeout" "30s"

# ── error cases ────────────────────────────────────────────────────────────

echo ""
echo -e "${YELLOW}--- error cases ---${NC}"
assert_fail "invalid syntax should fail" \
	"$CONFIGS/invalid_syntax.conf"
assert_fail "empty config should fail" \
	"$CONFIGS/empty.conf"
assert_fail "nonexistent file should fail" \
	"$CONFIGS/nonexistent.conf"

# ── summary ────────────────────────────────────────────────────────────────

echo ""
echo "=============================="
echo -e " Results: ${GREEN}$PASS passed${NC}, ${RED}$FAIL failed${NC}, $TOTAL total"
echo "=============================="
echo ""

exit $FAIL
