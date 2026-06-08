#!/usr/bin/env bash
# Phase 4: HTTP correctness — Host header, versions, methods, status codes.
set -u
cd "$(dirname "$0")/../.."
. test/system/lib.sh

H=localhost
P=8080

echo "=== Phase 4: HTTP correctness ==="

# --- Host header ---
echo "  -- Host header --"
# HTTP/1.1 without Host -> 400
out=$(printf 'GET / HTTP/1.1\r\n\r\n' | nc -w 3 $H $P 2>/dev/null | head -1)
code=$(echo "$out" | awk '{print $2}')
if [ "$code" = "400" ]; then PASS=$((PASS+1)); echo "  [PASS] HTTP/1.1 sans Host -> 400"; else FAIL=$((FAIL+1)); echo "  [FAIL] HTTP/1.1 sans Host -> $code (expected 400)"; fi

expect_status "HTTP/1.1 avec Host" 200 "http://$H:$P/"
expect_status "HTTP/1.0" 200 --http1.0 "http://$H:$P/"

# --- HTTP version invalid ---
echo "  -- HTTP versions --"
out=$(printf 'GET / HTTP/2.0\r\nHost: %s\r\n\r\n' "$H" | nc -w 3 $H $P 2>/dev/null | head -1)
code=$(echo "$out" | awk '{print $2}')
if [ "$code" = "505" ] || [ "$code" = "400" ] || [ "$code" = "501" ]; then PASS=$((PASS+1)); echo "  [PASS] HTTP/2.0 rejected -> $code"; else FAIL=$((FAIL+1)); echo "  [FAIL] HTTP/2.0 -> $code (expected 505/400/501)"; fi

out=$(printf 'GET / HTTP/9.9\r\nHost: %s\r\n\r\n' "$H" | nc -w 3 $H $P 2>/dev/null | head -1)
code=$(echo "$out" | awk '{print $2}')
if [ "$code" = "505" ] || [ "$code" = "400" ] || [ "$code" = "501" ]; then PASS=$((PASS+1)); echo "  [PASS] HTTP/9.9 rejected -> $code"; else FAIL=$((FAIL+1)); echo "  [FAIL] HTTP/9.9 -> $code (expected 505/400/501)"; fi

# --- Methods ---
echo "  -- Methods --"
expect_status "GET /" 200 "http://$H:$P/"
expect_status "HEAD /" 200 -I "http://$H:$P/"
expect_status_in "POST /uploads/dummy" "200|201|405" -X POST -d "x=1" "http://$H:$P/uploads/dummy"
# /api allows only GET POST
expect_status "GET /api -> 200|404 (no index)" 200 "http://$H:$P/api/"
# Wait - /api may not have content; check 404 acceptable
out=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 -X PATCH "http://$H:$P/")
case "$out" in
  400|405|501) PASS=$((PASS+1)); echo "  [PASS] PATCH / rejected -> $out" ;;
  *) FAIL=$((FAIL+1)); echo "  [FAIL] PATCH / -> $out (expected 400/405/501)" ;;
esac

# Unknown method
out=$(printf 'FOO / HTTP/1.1\r\nHost: %s\r\n\r\n' "$H" | nc -w 3 $H $P 2>/dev/null | head -1)
code=$(echo "$out" | awk '{print $2}')
if [ "$code" = "400" ] || [ "$code" = "405" ] || [ "$code" = "501" ]; then PASS=$((PASS+1)); echo "  [PASS] Unknown method FOO -> $code"; else FAIL=$((FAIL+1)); echo "  [FAIL] FOO method -> $code"; fi

# Method-not-allowed: DELETE on /api (config: GET POST only)
expect_status "DELETE /api -> 405" 405 -X DELETE "http://$H:$P/api/x"

# --- Mandatory response headers ---
echo "  -- Response headers --"
expect_header "Server header present" Server "" "http://$H:$P/"
expect_header "Date header present" Date "" "http://$H:$P/"
expect_header "Content-Type on /index_test.html" "Content-Type" "text/html" "http://$H:$P/index_test.html"

# --- URL & header limits ---
echo "  -- Limits --"
big=$(printf 'a%.0s' {1..9000})
expect_status_in "URL >9KB" "414|400" "http://$H:$P/$big"

# Header oversized (>1KB based on CLIENT_HEADER_BUFFER_SIZE)
big_hdr=$(printf 'X-Big: %s\r\n' "$(printf 'a%.0s' {1..2048})")
out=$(printf 'GET / HTTP/1.1\r\nHost: %s\r\n%s\r\n' "$H" "$big_hdr" | nc -w 3 $H $P 2>/dev/null | head -1)
code=$(echo "$out" | awk '{print $2}')
if [ "$code" = "400" ] || [ "$code" = "431" ] || [ "$code" = "413" ]; then PASS=$((PASS+1)); echo "  [PASS] Header oversize -> $code"; else FAIL=$((FAIL+1)); echo "  [FAIL] Header oversize -> $code"; fi

summary
exit $FAIL
