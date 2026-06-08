#!/usr/bin/env bash
# Phase 11: Keep-alive & connection lifecycle.
set -u
cd "$(dirname "$0")/../.."
. test/system/lib.sh

H=localhost; P=8080

echo "=== Phase 11: Keep-alive & lifecycle ==="

# 5 sequential requests on a single connection (curl --next reuses TCP)
echo "  -- Pipelined / re-used connection --"
out=$(curl -s -o /dev/null -w "%{http_code}\n" \
  "http://$H:$P/" --next "http://$H:$P/test.txt" \
  --next "http://$H:$P/index_test.html" --next "http://$H:$P/empty.txt" \
  --next "http://$H:$P/" 2>/dev/null)
oks=$(echo "$out" | grep -c '^200$' || echo 0)
if [ "$oks" -eq 5 ]; then PASS=$((PASS+1)); echo "  [PASS] 5 responses on 1 connection"; else FAIL=$((FAIL+1)); echo "  [FAIL] only $oks/5 OK ($out)"; fi

# Connection: close
echo "  -- Connection: close --"
out=$(printf 'GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n' "$H" | nc -w 3 $H $P 2>/dev/null)
if echo "$out" | grep -qi "Connection: close" || echo "$out" | grep -qi "Connection: Close"; then PASS=$((PASS+1)); echo "  [PASS] Connection: close echoed in response"; else FAIL=$((FAIL+1)); echo "  [FAIL] no Connection: close in response"; fi

# Default keep-alive header
echo "  -- Default keep-alive --"
expect_header "Connection: keep-alive default" "Connection" "keep-alive" "http://$H:$P/"

# 100 sequential reqs same connection (stress for keep-alive)
echo "  -- 100 reqs same connection --"
NEXTS=""
for i in $(seq 1 100); do NEXTS="$NEXTS --next http://$H:$P/?n=$i"; done
out=$(curl -s -o /dev/null -w "%{http_code}\n" "http://$H:$P/" $NEXTS 2>/dev/null)
oks=$(echo "$out" | grep -c '^200$' || echo 0)
if [ "$oks" -ge 100 ]; then PASS=$((PASS+1)); echo "  [PASS] 100+ requests on 1 connection ($oks/101)"; else FAIL=$((FAIL+1)); echo "  [FAIL] only $oks/101"; fi

# Survival
expect_status "GET / still alive" 200 "http://$H:$P/"

summary
exit $FAIL
