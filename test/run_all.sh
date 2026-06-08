#!/usr/bin/env bash
# Webserv exhaustive test battery — orchestrator
# Usage:
#   ./test/run_all.sh                  — run all phases
#   ./test/run_all.sh phase <number>   — run a single phase
#   ./test/run_all.sh --quick          — skip stress (13) and 42-tester (17)
#
# Output: test/results/<timestamp>/{phase}.log + summary.md
set -u
cd "$(dirname "$0")/.."
ROOT="$(pwd)"

H=localhost
P=8080
CONF="configs/blucken.conf"

TS=$(date +%Y%m%d-%H%M%S)
RESDIR="$ROOT/test/results/$TS"
mkdir -p "$RESDIR"
ln -sfn "$TS" "$ROOT/test/results/latest"

PIDFILE="$RESDIR/webserv.pid"
LOGFILE="$RESDIR/webserv.stdout.log"

PHASE_PASS=0
PHASE_FAIL=0
PHASE_SKIP=0
SUMMARY_LINES=()

color() { printf "\033[%sm%s\033[0m" "$1" "$2"; }
green()   { color "0;32" "$1"; }
red()     { color "0;31" "$1"; }
yellow()  { color "0;33" "$1"; }

# ---------- server lifecycle ----------
start_server() {
  local cfg="${1:-$CONF}"
  pkill -9 -f "./webserv" 2>/dev/null || true
  sleep 0.5
  nohup "$ROOT/webserv" "$cfg" > "$LOGFILE" 2>&1 &
  echo $! > "$PIDFILE"
  sleep 1.5
  if kill -0 "$(cat "$PIDFILE")" 2>/dev/null; then return 0; else return 1; fi
}
stop_server() {
  if [ -f "$PIDFILE" ]; then
    kill -9 "$(cat "$PIDFILE")" 2>/dev/null || true
    rm -f "$PIDFILE"
  fi
  pkill -9 -f "./webserv" 2>/dev/null || true
}

trap stop_server EXIT INT TERM

# ---------- helpers ----------
run_phase() {
  local num="$1" desc="$2"; shift 2
  local cmd="$*"
  local out="$RESDIR/${num}-$(echo "$desc" | tr ' /' '__').log"
  echo ""
  echo "=== Phase $num: $desc ==="
  if eval "$cmd" > "$out" 2>&1; then
    rc=0
    PHASE_PASS=$((PHASE_PASS+1))
    SUMMARY_LINES+=("- [PASS] Phase $num: $desc → \`$(basename "$out")\`")
    echo "  $(green "[PHASE PASS]") $desc"
  else
    rc=$?
    PHASE_FAIL=$((PHASE_FAIL+1))
    SUMMARY_LINES+=("- [FAIL] Phase $num: $desc (exit $rc) → \`$(basename "$out")\`")
    echo "  $(red "[PHASE FAIL]") $desc (exit $rc)"
  fi
  tail -5 "$out" | sed 's/^/    /'
  return $rc
}

skip_phase() {
  local num="$1" desc="$2" reason="$3"
  PHASE_SKIP=$((PHASE_SKIP+1))
  SUMMARY_LINES+=("- [SKIP] Phase $num: $desc — $reason")
  echo ""
  echo "=== Phase $num: $desc ==="
  echo "  $(yellow "[SKIP]") $reason"
}

# ---------- main pipeline ----------
ONLY_PHASE="${1:-}"
QUICK=0
for arg in "$@"; do
  [ "$arg" = "--quick" ] && QUICK=1
done

echo "Webserv exhaustive test battery — $TS"
echo "Results: $RESDIR"
echo ""

# Phase 0: prep
if [ -z "$ONLY_PHASE" ] || [ "$ONLY_PHASE" = "0" ]; then
  echo "=== Phase 0: preparation ==="
  pkill -9 -f "./webserv" 2>/dev/null || true
  rm -rf logs/*.log www/uploads/* www/webdav/uploads/* www/app/uploads/*.html 2>/dev/null
  touch www/uploads/.gitkeep www/webdav/uploads/.gitkeep www/app/uploads/.gitkeep 2>/dev/null || true
fi

# Phase 1: build
if [ -z "$ONLY_PHASE" ] || [ "$ONLY_PHASE" = "1" ]; then
  run_phase 1 "Build c++98" "make fclean >/dev/null && make all"
fi

# Phase 2: parser
if [ -z "$ONLY_PHASE" ] || [ "$ONLY_PHASE" = "2" ]; then
  run_phase 2 "Parser smoke" "./test/system/parser_smoke.sh"
fi

# Start server for HTTP phases
start_server || { echo "$(red "Cannot start webserv")"; exit 1; }
echo "  webserv started PID=$(cat $PIDFILE)"

# Phase 3: smoke multi-port
if [ -z "$ONLY_PHASE" ] || [ "$ONLY_PHASE" = "3" ]; then
  run_phase 3 "Smoke multi-port" '
    OK=0; for p in 8080 8081 8082 8083 8084; do
      c=$(curl -s -o /dev/null -w "%{http_code}" --max-time 3 "http://'"$H"':$p/")
      [ "$c" = "200" ] && OK=$((OK+1))
    done
    [ "$OK" -eq 5 ] && echo "5/5 ports up" || { echo "$OK/5 only"; exit 1; }
  '
fi

# Phases 4-14
for ph in 04_http 05_static 06_methods 07_upload 08_cgi 09_10_redirects_errors 11_keepalive 12_robust 14_sessions; do
  num="${ph%%_*}"
  if [ -z "$ONLY_PHASE" ] || [ "$ONLY_PHASE" = "$num" ]; then
    [ -x "test/system/${ph}.sh" ] && run_phase "$num" "$ph" "test/system/${ph}.sh"
  fi
done

# Phase 13: stress (skipped in --quick)
if [ -z "$ONLY_PHASE" ] || [ "$ONLY_PHASE" = "13" ]; then
  if [ "$QUICK" = "1" ]; then
    skip_phase 13 "Stress concurrency" "--quick mode"
  else
    [ -x "test/system/13_stress.sh" ] && run_phase 13 "Stress concurrency" "test/system/13_stress.sh"
  fi
fi

# Phase 15: NGINX comparison (optional)
if [ -z "$ONLY_PHASE" ] || [ "$ONLY_PHASE" = "15" ]; then
  if command -v nginx >/dev/null 2>&1; then
    run_phase 15 "NGINX comparison" "./test/nginx_tests/run-tests.sh" || true
  else
    skip_phase 15 "NGINX comparison" "nginx not in PATH"
  fi
fi

# Phase 16: sanitize rerun
if [ -z "$ONLY_PHASE" ] || [ "$ONLY_PHASE" = "16" ]; then
  if [ -x "test/system/16_sanitize.sh" ]; then
    stop_server
    run_phase 16 "ASan rebuild + smoke" "test/system/16_sanitize.sh"
    # restart normal binary if more phases follow
    [ -z "$ONLY_PHASE" ] && start_server
  fi
fi

# Phase 17: docker 42 tester
if [ -z "$ONLY_PHASE" ] || [ "$ONLY_PHASE" = "17" ]; then
  if [ "$QUICK" = "1" ]; then
    skip_phase 17 "42 tester (Docker)" "--quick mode"
  elif [ -x "test/system/17_docker_tester.sh" ]; then
    run_phase 17 "42 tester (Docker)" "test/system/17_docker_tester.sh" || true
  else
    skip_phase 17 "42 tester (Docker)" "script missing"
  fi
fi

stop_server

# ---------- summary ----------
SUMMARY="$RESDIR/summary.md"
{
  echo "# Webserv test battery — $TS"
  echo ""
  echo "**Result:** PASS=$PHASE_PASS  FAIL=$PHASE_FAIL  SKIP=$PHASE_SKIP"
  echo ""
  echo "## Phases"
  for line in "${SUMMARY_LINES[@]}"; do echo "$line"; done
  echo ""
  echo "## Logs"
  echo ""
  ls -1 "$RESDIR" | sed 's/^/- /'
} > "$SUMMARY"

echo ""
echo "============================================"
echo "  Summary: PASS=$PHASE_PASS  FAIL=$PHASE_FAIL  SKIP=$PHASE_SKIP"
echo "  Report: $SUMMARY"
echo "============================================"

[ "$PHASE_FAIL" -eq 0 ]
