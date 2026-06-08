#!/usr/bin/env bash
# Phase 5: Static file serving — MIME types, autoindex, default index, 404, path traversal.
set -u
cd "$(dirname "$0")/../.."
. test/system/lib.sh

H=localhost; P=8080

echo "=== Phase 5: Static serving ==="

# --- Existing files ---
echo "  -- Files --"
expect_status "GET /index_test.html" 200 "http://$H:$P/index_test.html"
expect_header "  text/html MIME" "Content-Type" "text/html" "http://$H:$P/index_test.html"

expect_status "GET /test.txt" 200 "http://$H:$P/test.txt"
expect_header "  text/plain MIME" "Content-Type" "text/plain" "http://$H:$P/test.txt"
expect_body_contains "  body of /test.txt" "hello" "http://$H:$P/test.txt"

expect_status "GET /empty.txt" 200 "http://$H:$P/empty.txt"
expect_header "  Content-Length: 0" "Content-Length" "0" "http://$H:$P/empty.txt"

# Big file integrity
expect_status "GET /hugefile.txt (2.7 MB)" 200 "http://$H:$P/hugefile.txt"
md_real=$(md5 -q www/hugefile.txt)
md_got=$(curl -s --max-time 30 "http://$H:$P/hugefile.txt" | md5 -q)
if [ "$md_real" = "$md_got" ]; then PASS=$((PASS+1)); echo "  [PASS] hugefile MD5 match"; else FAIL=$((FAIL+1)); echo "  [FAIL] hugefile MD5 mismatch ($md_got vs $md_real)"; fi

# Unknown extension -> default_type (text/html in blucken.conf)
expect_status "GET /file.xyz (unknown ext)" 200 "http://$H:$P/file.xyz"
expect_header "  default_type" "Content-Type" "text/html" "http://$H:$P/file.xyz"

# No extension -> default_type
expect_status "GET /noext (no ext)" 200 "http://$H:$P/noext"

# URL-encoded space
expect_status "GET /my%20file.txt (space encoded)" 200 "http://$H:$P/my%20file.txt"

# --- Default index ---
echo "  -- Default index --"
# /static has autoindex on, no index — should list dir
expect_status "GET /static/ (autoindex)" 200 "http://$H:$P/static/"
expect_body_contains "  /static body has links" "<a" "http://$H:$P/static/"

# Root has index.html, index.htm, default.html priority
expect_status "GET / (default index)" 200 "http://$H:$P/"

# --- 404 ---
echo "  -- 404 --"
expect_status "GET /missing.html -> 404" 404 "http://$H:$P/missing.html"
expect_body_contains "  /404.html custom body" "404" "http://$H:$P/missing.html"

# /api with no index, allowed_methods GET POST, but no autoindex
expect_status_in "GET /api/missing -> 404" "404" "http://$H:$P/api/missing"

# --- Path traversal ---
echo "  -- Path traversal --"
expect_status_in "GET /../../etc/passwd" "400|404|403" "http://$H:$P/../../etc/passwd"
expect_status_in "GET /%2e%2e%2fetc/passwd" "400|404|403" "http://$H:$P/%2e%2e%2fetc/passwd"
expect_status_in "GET /%2E%2E/%2E%2E/etc/passwd" "400|404|403" "http://$H:$P/%2E%2E/%2E%2E/etc/passwd"

# Body must NOT contain "root:" (i.e. /etc/passwd content)
body=$(curl -s --max-time 5 "http://$H:$P/../../etc/passwd")
if ! echo "$body" | grep -q "root:"; then PASS=$((PASS+1)); echo "  [PASS] No /etc/passwd leak"; else FAIL=$((FAIL+1)); echo "  [FAIL] /etc/passwd LEAKED!"; fi

summary
exit $FAIL
