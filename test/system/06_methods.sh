#!/usr/bin/env bash
# Phase 6: Methods & DAV — POST, PUT, DELETE, allow_methods, dav_methods.
set -u
cd "$(dirname "$0")/../.."
. test/system/lib.sh

H=localhost; P=8080
UPLOADS=www/uploads
WEBDAV=www/webdav/uploads

# Cleanup before starting
rm -f $UPLOADS/test.txt $UPLOADS/replace.txt $UPLOADS/dav-test.txt
rm -rf $UPLOADS/nested
rm -f $WEBDAV/dav.txt
rm -rf $WEBDAV/deep

echo "=== Phase 6: Methods & DAV ==="

# --- PUT ---
echo "  -- PUT --"
expect_status "PUT /uploads/test.txt (create)" 201 -X PUT --data "abc" -H "Content-Type: text/plain" "http://$H:$P/uploads/test.txt"
if [ -f "$UPLOADS/test.txt" ]; then PASS=$((PASS+1)); echo "  [PASS] file created on disk"; else FAIL=$((FAIL+1)); echo "  [FAIL] file NOT on disk"; fi
content=$(cat "$UPLOADS/test.txt" 2>/dev/null)
if [ "$content" = "abc" ]; then PASS=$((PASS+1)); echo "  [PASS] file content matches"; else FAIL=$((FAIL+1)); echo "  [FAIL] content='$content' (expected 'abc')"; fi

# Overwrite
expect_status_in "PUT /uploads/test.txt (overwrite)" "200|204" -X PUT --data "xyz" -H "Content-Type: text/plain" "http://$H:$P/uploads/test.txt"
content=$(cat "$UPLOADS/test.txt" 2>/dev/null)
if [ "$content" = "xyz" ]; then PASS=$((PASS+1)); echo "  [PASS] overwrite ok"; else FAIL=$((FAIL+1)); echo "  [FAIL] content='$content'"; fi

# Nested path with create_full_put_path on (global)
expect_status_in "PUT /uploads/nested/deep/file.txt" "201|200|204" -X PUT --data "deep" -H "Content-Type: text/plain" "http://$H:$P/uploads/nested/deep/file.txt"
if [ -f "$UPLOADS/nested/deep/file.txt" ]; then PASS=$((PASS+1)); echo "  [PASS] nested file created"; else FAIL=$((FAIL+1)); echo "  [FAIL] nested file missing"; fi

# /webdav route specifically with dav_methods PUT DELETE
expect_status_in "PUT /webdav/dav.txt" "201|200|204" -X PUT --data "dav" -H "Content-Type: text/plain" "http://$H:$P/webdav/dav.txt"

# --- DELETE ---
echo "  -- DELETE --"
expect_status_in "DELETE /uploads/test.txt" "200|204" -X DELETE "http://$H:$P/uploads/test.txt"
if [ ! -f "$UPLOADS/test.txt" ]; then PASS=$((PASS+1)); echo "  [PASS] file removed from disk"; else FAIL=$((FAIL+1)); echo "  [FAIL] file still on disk"; fi

expect_status "DELETE /uploads/missing.txt -> 404" 404 -X DELETE "http://$H:$P/uploads/missing.txt"

# DELETE webdav file
expect_status_in "DELETE /webdav/dav.txt" "200|204" -X DELETE "http://$H:$P/webdav/dav.txt"

# --- Method not allowed ---
echo "  -- 405 Allow header --"
# /static allows only GET HEAD
expect_status "DELETE /static/test.txt -> 405" 405 -X DELETE "http://$H:$P/static/test.txt"
expect_status "PUT /static/test.txt -> 405" 405 -X PUT --data x -H "Content-Type: text/plain" "http://$H:$P/static/test.txt"
expect_header "Allow header on /static 405" "Allow" "GET" -X DELETE "http://$H:$P/static/x"

# --- POST on a route ---
echo "  -- POST --"
# /uploads allows GET HEAD POST PUT DELETE
expect_status_in "POST /uploads/ with body" "200|201" -X POST --data "x=1" -H "Content-Type: application/x-www-form-urlencoded" "http://$H:$P/uploads/post.txt"

# Cleanup
rm -rf $UPLOADS/nested $UPLOADS/post.txt $UPLOADS/replace.txt 2>/dev/null

summary
exit $FAIL
