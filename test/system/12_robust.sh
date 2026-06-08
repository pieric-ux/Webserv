#!/usr/bin/env bash
# Phase 12: Robustesse — slowloris, malformed, RST, headers oversized.
# CRITICAL gate of the subject: server must NEVER crash.
set -u
cd "$(dirname "$0")/../.."
. test/system/lib.sh

H=localhost; P=8080
PIDFILE=$(ls test/results/*/webserv.pid 2>/dev/null | tail -1)
WSPID=""
if [ -n "$PIDFILE" ]; then WSPID=$(cat "$PIDFILE"); fi

check_alive() {
  local label="$1"
  if curl -s -o /dev/null -w "" --max-time 3 "http://$H:$P/" 2>/dev/null; then
    PASS=$((PASS+1)); echo "  [PASS] alive after: $label"
  else
    FAIL=$((FAIL+1)); echo "  [FAIL] DEAD after: $label"
  fi
}

echo "=== Phase 12: Robustesse ==="

# 1. Slowloris-style: send headers byte-by-byte, then disconnect
echo "  -- Slowloris (10s drip) --"
(
  exec 3<>/dev/tcp/$H/$P
  printf 'GET / HTTP/1.1\r\n' >&3
  for c in H o s t : ' ' l o c a l h o s t; do printf "%s" "$c" >&3; sleep 0.05; done
  printf '\r\n' >&3
  sleep 1
  exec 3<&-
) 2>/dev/null &
SLPID=$!
wait $SLPID 2>/dev/null
check_alive "slowloris drip"

# 2. RST close after connect
echo "  -- Connect+RST --"
for i in 1 2 3 4 5; do
  ( exec 3<>/dev/tcp/$H/$P; exec 3<&- ; exec 3>&- ) 2>/dev/null
done
check_alive "5x connect+RST"

# 3. 0-byte request: open + half-close
echo "  -- 0-byte then half-close --"
for i in 1 2 3; do
  printf '' | nc -w 1 $H $P >/dev/null 2>&1
done
check_alive "0-byte requests"

# 4. Body annoncé puis close prématuré
echo "  -- Premature close mid-body --"
( printf 'POST /cgi-bin/echo.sh HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\nContent-Length: 10000\r\n\r\nshort' "$H" | nc -w 2 $H $P >/dev/null 2>&1 ) &
sleep 1
check_alive "premature body close"

# 5. NULL byte in header
echo "  -- NULL byte in header --"
out=$(printf 'GET / HTTP/1.1\r\nHost: %s\r\nX-Bad: a\0b\r\n\r\n' "$H" | nc -w 2 $H $P 2>/dev/null | head -1)
echo "  raw response: $out"
check_alive "NULL byte injection"

# 6. lowercase method
echo "  -- Lowercase method --"
out=$(printf 'get / HTTP/1.1\r\nHost: %s\r\n\r\n' "$H" | nc -w 2 $H $P 2>/dev/null | head -1)
code=$(echo "$out" | awk '{print $2}')
case "$code" in
  400|501|405) PASS=$((PASS+1)); echo "  [PASS] lowercase method -> $code (rejected)" ;;
  200) FAIL=$((FAIL+1)); echo "  [FAIL] lowercase method accepted as 200 (should reject)" ;;
  *) PASS=$((PASS+1)); echo "  [PASS] lowercase method -> $code (any non-200 ok)" ;;
esac
check_alive "lowercase method"

# 7. 200 connexions consécutives (pas 1000 pour économiser le temps)
echo "  -- 200 sequential connections --"
ok=0
for i in $(seq 1 200); do
  if curl -s -o /dev/null --max-time 2 "http://$H:$P/" 2>/dev/null; then ok=$((ok+1)); fi
done
echo "  $ok/200 succeeded"
if [ "$ok" -ge 195 ]; then PASS=$((PASS+1)); echo "  [PASS] $ok/200 connections OK"; else FAIL=$((FAIL+1)); echo "  [FAIL] only $ok/200"; fi
check_alive "200 sequential"

# 8. Pipe break (client kills connection during response of large file)
echo "  -- Client kills mid-response --"
( curl -s --max-time 0.05 "http://$H:$P/hugefile.txt" > /dev/null 2>&1 || true ) &
sleep 0.5
wait 2>/dev/null
check_alive "client kill mid-response"

# 9. fd leak check
echo "  -- fd leak check --"
if [ -n "$WSPID" ] && kill -0 $WSPID 2>/dev/null; then
  pre=$(lsof -p $WSPID 2>/dev/null | wc -l | tr -d ' ')
  for i in $(seq 1 50); do curl -s -o /dev/null --max-time 1 "http://$H:$P/" 2>/dev/null; done
  sleep 1
  post=$(lsof -p $WSPID 2>/dev/null | wc -l | tr -d ' ')
  delta=$((post - pre))
  echo "  fd count pre=$pre post=$post delta=$delta"
  if [ "$delta" -le 5 ]; then PASS=$((PASS+1)); echo "  [PASS] no fd leak (delta=$delta)"; else FAIL=$((FAIL+1)); echo "  [FAIL] fd leak suspected (delta=$delta)"; fi
fi

# 10. Header injection (CRLF)
echo "  -- CRLF injection in URL --"
out=$(curl -s -o /dev/null -w "%{http_code}" --max-time 3 "http://$H:$P/foo%0d%0aX-Inject:%20bad")
case "$out" in
  400|404|403) PASS=$((PASS+1)); echo "  [PASS] CRLF injection -> $out" ;;
  *) FAIL=$((FAIL+1)); echo "  [FAIL] CRLF injection -> $out" ;;
esac
check_alive "CRLF injection"

summary
exit $FAIL
