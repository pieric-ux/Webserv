#!/usr/bin/env bash
# Phase 15: Compare webserv (8081) vs nginx (8090) on identical paths.
# nginx must be running with /tmp/nginx-compare.conf for this test to be meaningful.
set -u
cd "$(dirname "$0")/../.."
. test/system/lib.sh

WS_PORT=8081
NX_PORT=8090
H=localhost

# Skip if nginx not running
if ! curl -s -o /dev/null --max-time 2 "http://$H:$NX_PORT/" 2>/dev/null; then
  echo "  [SKIP] nginx not running on $NX_PORT — start with: nginx -c /tmp/nginx-compare.conf -p /tmp/"
  exit 0
fi

echo "=== Phase 15: webserv vs nginx (status code parity) ==="

cmp() {
  local label="$1" path="$2"
  local ws nx
  ws=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "http://$H:$WS_PORT$path")
  nx=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "http://$H:$NX_PORT$path")
  if [ "$ws" = "$nx" ]; then
    PASS=$((PASS+1))
    printf "  [MATCH] %-30s ws=%s  nx=%s  -> %s\n" "$label" "$ws" "$nx" "$path"
  else
    FAIL=$((FAIL+1))
    printf "  [DIFF]  %-30s ws=%s  nx=%s  -> %s\n" "$label" "$ws" "$nx" "$path"
  fi
}

cmp "GET /" /
cmp "GET /test.txt" /test.txt
cmp "GET /index_test.html" /index_test.html
cmp "GET /missing" /missing-page-xyz
cmp "GET /static/" /static/
cmp "GET /file.xyz" /file.xyz
cmp "GET /noext" /noext
cmp "GET /old-page (301)" /old-page
cmp "GET /legacy (302)" /legacy
cmp "GET /gone (410)" /gone
cmp "GET /empty.txt" /empty.txt
cmp "GET path traversal" /../etc/passwd

# Compare Content-Length on a known file
ws_len=$(curl -s -D - -o /dev/null --max-time 3 "http://$H:$WS_PORT/test.txt" | awk -v IGNORECASE=1 '/^Content-Length:/ {print $2}' | tr -d '\r')
nx_len=$(curl -s -D - -o /dev/null --max-time 3 "http://$H:$NX_PORT/test.txt" | awk -v IGNORECASE=1 '/^Content-Length:/ {print $2}' | tr -d '\r')
if [ "$ws_len" = "$nx_len" ]; then
  PASS=$((PASS+1)); echo "  [MATCH] Content-Length test.txt: ws=$ws_len nx=$nx_len"
else
  FAIL=$((FAIL+1)); echo "  [DIFF] Content-Length: ws=$ws_len nx=$nx_len"
fi

summary
exit $FAIL
