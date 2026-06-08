#!/usr/bin/env bash
# Phases 9 & 10: Redirects (301/302/410) + custom error pages.
set -u
cd "$(dirname "$0")/../.."
. test/system/lib.sh

H=localhost; P=8080

echo "=== Phase 9: Redirects ==="

# /old-page -> 301 example.com/new-page
expect_status "GET /old-page -> 301" 301 "http://$H:$P/old-page"
expect_header "  Location external" "Location" "example.com/new-page" "http://$H:$P/old-page"

# /legacy -> 302 /modern
expect_status "GET /legacy -> 302" 302 "http://$H:$P/legacy"
expect_header "  Location internal" "Location" "/modern" "http://$H:$P/legacy"

# /gone -> 410
expect_status "GET /gone -> 410" 410 "http://$H:$P/gone"

# Curl follow
expect_status "GET /old-page with -L (followed)" 200 -L --max-redirs 0 -o /dev/null "http://$H:$P/old-page" || true

echo ""
echo "=== Phase 10: Error pages ==="

# 404 on / -> custom /404.html
body=$(curl -s --max-time 5 "http://$H:$P/missing-page-xyz")
if echo "$body" | grep -qi "404"; then PASS=$((PASS+1)); echo "  [PASS] 404 body has '404'"; else FAIL=$((FAIL+1)); echo "  [FAIL] 404 body: $(echo $body | head -c 100)"; fi

# /api/missing -> 404 with /api/not-found.html (location override)
body=$(curl -s --max-time 5 "http://$H:$P/api/missing")
if echo "$body" | grep -qiE "not.found|api"; then PASS=$((PASS+1)); echo "  [PASS] /api 404 (location-overridden)"; else FAIL=$((FAIL+1)); echo "  [FAIL] /api 404 body: $(echo $body | head -c 100)"; fi

# 50x via CGI exit-with-error: malformed.py triggers 502
body=$(curl -s --max-time 5 "http://$H:$P/cgi-bin/malformed.py")
if echo "$body" | grep -qE "50[0-9]"; then PASS=$((PASS+1)); echo "  [PASS] 502 body shown"; else FAIL=$((FAIL+1)); echo "  [FAIL] 502 body: $(echo $body | head -c 100)"; fi

# Default 50x.html exists in www/
[ -f www/50x.html ] && { PASS=$((PASS+1)); echo "  [PASS] /50x.html present"; } || { FAIL=$((FAIL+1)); echo "  [FAIL] /50x.html missing"; }
[ -f www/404.html ] && { PASS=$((PASS+1)); echo "  [PASS] /404.html present"; } || { FAIL=$((FAIL+1)); echo "  [FAIL] /404.html missing"; }

# 405 should also have a body (default page)
body=$(curl -s --max-time 5 -X DELETE "http://$H:$P/static/test.txt")
if [ -n "$body" ] && echo "$body" | grep -qi "405\|not allow"; then PASS=$((PASS+1)); echo "  [PASS] 405 body present"; else FAIL=$((FAIL+1)); echo "  [FAIL] 405 body empty/wrong: $(echo $body | head -c 80)"; fi

summary
exit $FAIL
