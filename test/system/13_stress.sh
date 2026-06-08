#!/usr/bin/env bash
# Phase 13: Stress concurrency — ab benchmark.
set -u
cd "$(dirname "$0")/../.."
. test/system/lib.sh

H=localhost; P=8080
PIDFILE=$(ls test/results/*/webserv.pid 2>/dev/null | tail -1)
WSPID=""
if [ -n "$PIDFILE" ]; then WSPID=$(cat "$PIDFILE"); fi

echo "=== Phase 13: Stress concurrency ==="

if [ -n "$WSPID" ] && kill -0 $WSPID 2>/dev/null; then
  pre_rss=$(ps -p $WSPID -o rss= | tr -d ' ')
  pre_fd=$(lsof -p $WSPID 2>/dev/null | wc -l | tr -d ' ')
  echo "  Pre-stress: RSS=${pre_rss}KB FD=${pre_fd}"
fi

echo "  -- ab -n 5000 -c 50 GET / --"
ab -q -n 5000 -c 50 "http://$H:$P/" > /tmp/ab1.log 2>&1
complete=$(grep "Complete requests:" /tmp/ab1.log | awk '{print $3}')
failed=$(grep "Failed requests:" /tmp/ab1.log | awk '{print $3}')
non2xx=$(grep "Non-2xx responses:" /tmp/ab1.log | awk '{print $3}')
non2xx=${non2xx:-0}
rps=$(grep "Requests per second:" /tmp/ab1.log | awk '{print $4}')
p99=$(grep "  99%" /tmp/ab1.log | awk '{print $2}')
echo "    completed=$complete failed=$failed non2xx=$non2xx rps=$rps p99=${p99}ms"
if [ "$complete" = "5000" ] && [ "$failed" = "0" ] && [ "$non2xx" = "0" -o -z "$non2xx" ]; then
  PASS=$((PASS+1)); echo "  [PASS] ab GET 5000 conc 50 — 100% success"
else
  FAIL=$((FAIL+1)); echo "  [FAIL] failures detected"
fi

echo "  -- ab -n 1000 -c 20 GET /test.txt (bigger conn pool) --"
ab -q -n 1000 -c 20 "http://$H:$P/test.txt" > /tmp/ab2.log 2>&1
complete=$(grep "Complete requests:" /tmp/ab2.log | awk '{print $3}')
failed=$(grep "Failed requests:" /tmp/ab2.log | awk '{print $3}')
echo "    completed=$complete failed=$failed"
if [ "$complete" = "1000" ] && [ "$failed" = "0" ]; then
  PASS=$((PASS+1)); echo "  [PASS] ab GET small file"
else
  FAIL=$((FAIL+1)); echo "  [FAIL]"
fi

echo "  -- ab -n 200 -c 10 GET /cgi-bin/hello.py (CGI under load) --"
ab -q -n 200 -c 10 "http://$H:$P/cgi-bin/hello.py" > /tmp/ab3.log 2>&1
complete=$(grep "Complete requests:" /tmp/ab3.log | awk '{print $3}')
failed=$(grep "Failed requests:" /tmp/ab3.log | awk '{print $3}')
non2xx=$(grep "Non-2xx responses:" /tmp/ab3.log | awk '{print $3}')
non2xx=${non2xx:-0}
echo "    completed=$complete failed=$failed non2xx=$non2xx"
# CGI under load can have some non-2xx if connections drop, accept up to 5%
if [ "$complete" = "200" ]; then
  if [ "$failed" = "0" ] || [ "$failed" -le 10 ]; then
    PASS=$((PASS+1)); echo "  [PASS] ab CGI 200 reqs (failed=$failed allowed <=10)"
  else
    FAIL=$((FAIL+1)); echo "  [FAIL] too many CGI failures"
  fi
fi

# Big file under load
echo "  -- ab -n 100 -c 10 GET /hugefile.txt (2.7MB file) --"
ab -q -n 100 -c 10 "http://$H:$P/hugefile.txt" > /tmp/ab4.log 2>&1
complete=$(grep "Complete requests:" /tmp/ab4.log | awk '{print $3}')
failed=$(grep "Failed requests:" /tmp/ab4.log | awk '{print $3}')
echo "    completed=$complete failed=$failed"
if [ "$complete" = "100" ] && [ "$failed" -le 5 ]; then
  PASS=$((PASS+1)); echo "  [PASS] ab big file"
else
  FAIL=$((FAIL+1)); echo "  [FAIL]"
fi

if [ -n "$WSPID" ] && kill -0 $WSPID 2>/dev/null; then
  sleep 1
  post_rss=$(ps -p $WSPID -o rss= | tr -d ' ')
  post_fd=$(lsof -p $WSPID 2>/dev/null | wc -l | tr -d ' ')
  echo "  Post-stress: RSS=${post_rss}KB FD=${post_fd}"
  rss_delta=$((post_rss - pre_rss))
  fd_delta=$((post_fd - pre_fd))
  echo "  Delta: RSS=+${rss_delta}KB FD=+${fd_delta}"
  if [ "$fd_delta" -le 10 ]; then PASS=$((PASS+1)); echo "  [PASS] FD stable"; else FAIL=$((FAIL+1)); echo "  [FAIL] FD leak"; fi
  # RSS may grow but should not double
  if [ "$rss_delta" -le 102400 ]; then PASS=$((PASS+1)); echo "  [PASS] RSS reasonable (+${rss_delta}KB)"; else FAIL=$((FAIL+1)); echo "  [FAIL] RSS grew >100MB"; fi
fi

# Final survival
expect_status "GET / still alive" 200 "http://$H:$P/"

summary
exit $FAIL
