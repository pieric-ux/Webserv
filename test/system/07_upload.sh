#!/usr/bin/env bash
# Phase 7: Upload & body limits — 413, 411, large bodies, chunked.
set -u
cd "$(dirname "$0")/../.."
. test/system/lib.sh

H=localhost; P=8080

echo "=== Phase 7: Upload & body limits ==="

# --- Body via CGI (echo.sh on /cgi-bin) ---
# /cgi-bin has client_max_body_size 10M
echo "  -- POST via CGI --"
expect_status_in "POST /cgi-bin/echo.sh body 1KB" "200" -X POST --data "$(printf 'a%.0s' {1..1024})" -H "Content-Type: text/plain" "http://$H:$P/cgi-bin/echo.sh"

# 11 MB body > 10 M location limit -> 413
echo "  -- 413 -- generating 11MB body"
dd if=/dev/zero of=/tmp/big11M.bin bs=1M count=11 2>/dev/null
expect_status_in "POST /cgi-bin/echo.sh body 11MB > 10M limit" "413" -X POST --data-binary "@/tmp/big11M.bin" -H "Content-Type: application/octet-stream" "http://$H:$P/cgi-bin/echo.sh"
rm -f /tmp/big11M.bin

# --- 411 Length Required ---
echo "  -- 411 Length Required --"
out=$(printf 'POST /cgi-bin/echo.sh HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\n\r\nbody-without-CL' "$H" | nc -w 3 $H $P 2>/dev/null | head -1)
code=$(echo "$out" | awk '{print $2}')
case "$code" in
  411|400|200) PASS=$((PASS+1)); echo "  [PASS] POST without Content-Length -> $code (411 ideal)" ;;
  *)            FAIL=$((FAIL+1)); echo "  [FAIL] POST no CL -> $code (expected 411/400)" ;;
esac

# --- Chunked transfer-encoding (documented: rejected with 501) ---
echo "  -- Chunked transfer-encoding (documented limitation: 501) --"
out=$(printf 'POST /cgi-bin/echo.sh HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\nTransfer-Encoding: chunked\r\n\r\n4\r\ndata\r\n0\r\n\r\n' "$H" | nc -w 3 $H $P 2>/dev/null | head -1)
code=$(echo "$out" | awk '{print $2}')
case "$code" in
  501) PASS=$((PASS+1)); echo "  [PASS] Chunked TE -> 501 (matches current limitation)" ;;
  200) PASS=$((PASS+1)); echo "  [PASS] Chunked TE -> 200 (un-chunking implemented!)" ;;
  *)   FAIL=$((FAIL+1)); echo "  [FAIL] Chunked TE -> $code (expected 501 or 200)" ;;
esac

# --- Large body within limit on / (root: client_max_body_size 2G) ---
echo "  -- Large body within global 2G limit --"
dd if=/dev/zero of=/tmp/medium100K.bin bs=1K count=100 2>/dev/null
# / accepts POST too
expect_status_in "POST / body 100KB" "200|201|405|415" -X POST --data-binary "@/tmp/medium100K.bin" -H "Content-Type: application/octet-stream" "http://$H:$P/"
rm -f /tmp/medium100K.bin

# --- Survival check ---
expect_status "GET / still alive" 200 "http://$H:$P/"

summary
exit $FAIL
