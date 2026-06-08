#!/usr/bin/env bash
# Phase 14: Sessions & cookies (bonus).
set -u
cd "$(dirname "$0")/../.."
. test/system/lib.sh

H=localhost; P=8081

echo "=== Phase 14: Sessions & cookies ==="

# Probe Set-Cookie on root
hdrs=$(curl -s -D - -o /dev/null --max-time 5 "http://$H:$P/" 2>/dev/null)
if echo "$hdrs" | grep -qi "Set-Cookie:"; then
  PASS=$((PASS+1))
  echo "  [PASS] Set-Cookie present on /"
  cookie=$(echo "$hdrs" | grep -i "Set-Cookie:" | head -1 | sed 's/Set-Cookie:[[:space:]]*//I' | tr -d '\r\n')
  echo "    cookie: $cookie"
else
  echo "  [INFO] no Set-Cookie on / (may need a CGI-driven session)"
fi

# Probe via CGI
hdrs=$(curl -s -D - -o /dev/null --max-time 5 "http://$H:$P/cgi-bin/hello.py" 2>/dev/null)
if echo "$hdrs" | grep -qi "Set-Cookie:"; then
  PASS=$((PASS+1)); echo "  [PASS] Set-Cookie present on CGI"
else
  echo "  [INFO] no Set-Cookie on CGI"
fi

# Send a Cookie header back; server should not crash and should accept it
echo "  -- Send custom Cookie --"
expect_status_in "GET / with Cookie: SESSIONID=ABC123" "200" -H "Cookie: SESSIONID=ABC123" "http://$H:$P/"

# Try a CGI that echoes cookies — env.sh shows HTTP_COOKIE env var
body=$(curl -s --max-time 5 -H "Cookie: TASTE=lemon" "http://$H:$P/cgi-bin/env.sh")
if echo "$body" | grep -qE "HTTP_COOKIE.*TASTE=lemon"; then
  PASS=$((PASS+1)); echo "  [PASS] HTTP_COOKIE forwarded to CGI"
else
  FAIL=$((FAIL+1)); echo "  [FAIL] HTTP_COOKIE not in CGI env"
  echo "    grepping for HTTP_COOKIE..."
  echo "$body" | grep -i cookie | head -2 || echo "    no HTTP_COOKIE found"
fi

# Multiple cookies in one Cookie header
expect_status_in "Multi-cookie request" "200" -H "Cookie: a=1; b=2; c=3" "http://$H:$P/"

# Survival
expect_status "GET / still alive" 200 "http://$H:$P/"

summary
exit $FAIL
