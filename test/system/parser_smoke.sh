#!/usr/bin/env bash
# Parser smoke test: each config must either start (timeout 124) or fail-fast non-zero.
# Invalid configs MUST exit non-zero before the timeout fires.
set -u
cd "$(dirname "$0")/../.."

CFG_DIR="test/parser_tests/configs"
PASS=0
FAIL=0

# Each line: "config_filename expected"
#   expected = "ok"       → must reach the event loop (exit 124 from timeout)
#   expected = "fail"     → must exit non-zero before the timeout fires (parser rejects)
declare -a CASES=(
  "full.conf ok"
  "minimal.conf ok"
  "comment.conf ok"
  "types_only.conf ok"
  "cgi_only_block.conf ok"
  "cgi_only_dir.conf ok"
  "dav_off.conf ok"
  "error_page_5xx.conf ok"
  "error_page_xxx.conf ok"
  "size_bytes.conf ok"
  "size_mega.conf ok"
  "size_giga.conf ok"
  "empty.conf fail"
  "invalid_syntax.conf fail"
)

for entry in "${CASES[@]}"; do
  cfg=$(echo "$entry" | awk '{print $1}')
  expect=$(echo "$entry" | awk '{print $2}')
  out=$(timeout 1s ./webserv "$CFG_DIR/$cfg" 2>&1)
  rc=$?
  status="?"
  if [ "$expect" = "ok" ]; then
    if [ "$rc" -eq 124 ]; then
      status="PASS"
      PASS=$((PASS+1))
    else
      status="FAIL (exit $rc, expected timeout 124)"
      FAIL=$((FAIL+1))
    fi
  else  # fail
    if [ "$rc" -ne 0 ] && [ "$rc" -ne 124 ]; then
      status="PASS"
      PASS=$((PASS+1))
    else
      status="FAIL (exit $rc, expected non-zero non-timeout)"
      FAIL=$((FAIL+1))
    fi
  fi
  printf "  [%-40s] %-4s  -> %s\n" "$cfg" "$expect" "$status"
done

# Also test all real configs/*.conf (only "ok" expected)
echo ""
echo "  --- full project configs ---"
for cfg in configs/blucken.conf configs/default.conf configs/pieric.conf configs/pieric_linux.conf configs/webserv.conf configs/webserv_full.conf example_configs/*.conf; do
  [ -f "$cfg" ] || continue
  rc=$(timeout 1s ./webserv "$cfg" >/dev/null 2>&1 ; echo $?)
  if [ "$rc" -eq 124 ]; then
    status="PASS"
    PASS=$((PASS+1))
  else
    status="FAIL (exit $rc)"
    FAIL=$((FAIL+1))
  fi
  printf "  [%-40s]       -> %s\n" "$(basename "$cfg")" "$status"
done

echo ""
echo "  PASS=$PASS  FAIL=$FAIL"
[ "$FAIL" -eq 0 ]
