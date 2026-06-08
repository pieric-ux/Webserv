#!/usr/bin/env bash
# Phase 16: ASan rerun of critical paths.
# Pre-requisite: webserv must be the ASan binary, already running.
set -u
cd "$(dirname "$0")/../.."
. test/system/lib.sh

H=localhost; P=8081

echo "=== Phase 16: ASan critical-path replay ==="

# 1. Static + 404 + autoindex
expect_status "GET /" 200 "http://$H:$P/"
expect_status "GET /test.txt" 200 "http://$H:$P/test.txt"
expect_status "GET /missing -> 404 custom" 404 "http://$H:$P/missing"
expect_status "GET /static/" 200 "http://$H:$P/static/"
expect_status "GET /hugefile.txt 2.7MB" 200 "http://$H:$P/hugefile.txt"

# 2. CGI
expect_status "GET /cgi-bin/hello.py" 200 "http://$H:$P/cgi-bin/hello.py"
expect_status "POST /cgi-bin/echo.sh" 200 -X POST --data "asan-test" -H "Content-Type: text/plain" "http://$H:$P/cgi-bin/echo.sh"
expect_status_in "malformed.py -> 502" "502|500" "http://$H:$P/cgi-bin/malformed.py"

# 3. 50 parallel reqs
echo "  -- 50 parallel reqs --"
seq 1 50 | xargs -P 10 -I{} curl -s -o /dev/null -w "%{http_code}\n" --max-time 5 "http://$H:$P/" > /tmp/asan-par.log
oks=$(grep -c '^200$' /tmp/asan-par.log || echo 0)
if [ "$oks" -eq 50 ]; then PASS=$((PASS+1)); echo "  [PASS] 50/50 OK"; else FAIL=$((FAIL+1)); echo "  [FAIL] $oks/50"; fi

# 4. Slowloris under ASan
echo "  -- Slowloris (drip) --"
(
  exec 3<>/dev/tcp/$H/$P
  printf 'GET / HTTP/1.1\r\n' >&3
  for c in H o s t : ' ' l o c a l h o s t; do printf "%s" "$c" >&3; sleep 0.05; done
  printf '\r\n' >&3
  sleep 1
  exec 3<&-
) 2>/dev/null
expect_status "GET / after slowloris (alive)" 200 "http://$H:$P/"

# 5. NULL byte
out=$(printf 'GET / HTTP/1.1\r\nHost: %s\r\nX-Bad: a\0b\r\n\r\n' "$H" | nc -w 2 $H $P 2>/dev/null | head -1)
code=$(echo "$out" | awk '{print $2}')
if [ "$code" = "400" ]; then PASS=$((PASS+1)); echo "  [PASS] NULL byte -> 400"; else FAIL=$((FAIL+1)); echo "  [FAIL] NULL -> $code"; fi

# 6. CRLF injection
expect_status_in "CRLF injection URL" "400|404" "http://$H:$P/foo%0d%0aX-Bad:%20yo"

# 7. URL too long
big=$(printf 'a%.0s' {1..9000})
expect_status_in "Big URL" "414|400" "http://$H:$P/$big"

# 8. Sessions/cookies
hdrs=$(curl -s -D - -o /dev/null --max-time 3 "http://$H:$P/" 2>/dev/null)
if echo "$hdrs" | grep -qi "Set-Cookie"; then PASS=$((PASS+1)); echo "  [PASS] cookies still emitted"; else FAIL=$((FAIL+1)); echo "  [FAIL] no cookies"; fi

# Final survival
expect_status "GET / final survival" 200 "http://$H:$P/"

summary
exit $FAIL
