#!/usr/bin/env bash
# Phase 8: CGI — Python, Bash, env vars, query string, POST body, timeout, malformed.
set -u
cd "$(dirname "$0")/../.."
. test/system/lib.sh

H=localhost; P=8080

echo "=== Phase 8: CGI ==="

echo "  -- Basic CGI --"
expect_status "GET /cgi-bin/hello.py" 200 "http://$H:$P/cgi-bin/hello.py"
expect_body_contains "  hello.py body" "Hello from CGI" "http://$H:$P/cgi-bin/hello.py"
expect_header "  hello.py text/html" "Content-Type" "text/html" "http://$H:$P/cgi-bin/hello.py"

# Bash CGI
expect_status "GET /cgi-bin/env.sh" 200 "http://$H:$P/cgi-bin/env.sh"
expect_body_contains "  env.sh has REQUEST_METHOD" "REQUEST_METHOD" "http://$H:$P/cgi-bin/env.sh"

echo "  -- Query string passing --"
body=$(curl -s --max-time 8 "http://$H:$P/cgi-bin/env.sh?name=foo&val=bar")
if echo "$body" | grep -q "QUERY_STRING=name=foo&val=bar"; then PASS=$((PASS+1)); echo "  [PASS] QUERY_STRING transmitted"; else FAIL=$((FAIL+1)); echo "  [FAIL] QUERY_STRING missing"; echo "$body" | grep -i query | head -3; fi

# Required CGI env vars
echo "  -- CGI env vars --"
body=$(curl -s --max-time 8 "http://$H:$P/cgi-bin/env.sh")
for v in REQUEST_METHOD SERVER_PROTOCOL SERVER_SOFTWARE GATEWAY_INTERFACE SCRIPT_NAME SERVER_NAME SERVER_PORT REMOTE_ADDR PATH_INFO; do
  if echo "$body" | grep -q "^$v="; then PASS=$((PASS+1)); echo "  [PASS] env $v present"; else FAIL=$((FAIL+1)); echo "  [FAIL] env $v missing"; fi
done

echo "  -- POST body via stdin --"
expect_body_contains "POST echo.sh sees body" "hello-cgi" -X POST --data "hello-cgi" -H "Content-Type: text/plain" "http://$H:$P/cgi-bin/echo.sh"

# Bigger POST body
echo "  -- POST 100KB to CGI --"
dd if=/dev/zero bs=1K count=100 2>/dev/null | base64 > /tmp/cgi-body.txt
sz=$(wc -c < /tmp/cgi-body.txt | tr -d ' ')
ret=$(curl -s --max-time 30 -X POST --data-binary "@/tmp/cgi-body.txt" -H "Content-Type: text/plain" "http://$H:$P/cgi-bin/echo.sh" | wc -c | tr -d ' ')
if [ "$ret" -ge "$sz" ]; then PASS=$((PASS+1)); echo "  [PASS] CGI received $ret bytes (sent $sz)"; else FAIL=$((FAIL+1)); echo "  [FAIL] CGI received $ret bytes (expected ~$sz)"; fi
rm -f /tmp/cgi-body.txt

echo "  -- CGI timeout (sleep 60s, server timeout 30s) --"
start=$(date +%s)
code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 50 "http://$H:$P/cgi-bin/sleep.py")
elapsed=$(( $(date +%s) - start ))
case "$code" in
  504|500|502) PASS=$((PASS+1)); echo "  [PASS] sleep.py timed out -> $code (${elapsed}s elapsed)" ;;
  *) FAIL=$((FAIL+1)); echo "  [FAIL] sleep.py -> $code after ${elapsed}s" ;;
esac

echo "  -- Malformed CGI output --"
expect_status_in "malformed.py -> 502/500" "502|500" "http://$H:$P/cgi-bin/malformed.py"

echo "  -- 10 parallel CGI --"
PIDS=()
for i in 1 2 3 4 5 6 7 8 9 10; do
  curl -s -o /dev/null -w "%{http_code}\n" --max-time 10 "http://$H:$P/cgi-bin/hello.py?run=$i" >> /tmp/cgi-par.log &
  PIDS+=($!)
done
for p in "${PIDS[@]}"; do wait "$p" 2>/dev/null; done
oks=$(grep -c '^200$' /tmp/cgi-par.log 2>/dev/null || echo 0)
if [ "$oks" -eq 10 ]; then PASS=$((PASS+1)); echo "  [PASS] 10/10 parallel CGI all 200"; else FAIL=$((FAIL+1)); echo "  [FAIL] only $oks/10 OK"; fi
rm -f /tmp/cgi-par.log

# Survival
expect_status "GET / still alive" 200 "http://$H:$P/"

summary
exit $FAIL
