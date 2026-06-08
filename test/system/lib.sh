#!/usr/bin/env bash
# Common helpers for system tests. Source from each phase script.

PASS=${PASS:-0}
FAIL=${FAIL:-0}
RESULTS=()

# expect_status <label> <expected_code> <curl-args...>
expect_status() {
  local label="$1"; shift
  local expect="$1"; shift
  local code
  code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 8 "$@" 2>/dev/null)
  if [ "$code" = "$expect" ]; then
    PASS=$((PASS+1))
    printf "  [PASS] %-60s -> %s\n" "$label" "$code"
  else
    FAIL=$((FAIL+1))
    printf "  [FAIL] %-60s -> got %s expected %s\n" "$label" "$code" "$expect"
  fi
}

# expect_status_in <label> <"200|201"> <curl-args...>
expect_status_in() {
  local label="$1"; shift
  local expects="$1"; shift
  local code
  code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 8 "$@" 2>/dev/null)
  if echo "$expects" | tr '|' '\n' | grep -qx "$code"; then
    PASS=$((PASS+1))
    printf "  [PASS] %-60s -> %s\n" "$label" "$code"
  else
    FAIL=$((FAIL+1))
    printf "  [FAIL] %-60s -> got %s expected one of %s\n" "$label" "$code" "$expects"
  fi
}

# expect_header <label> <header-name> <expected-substring> <curl-args...>
expect_header() {
  local label="$1"; shift
  local hdr="$1"; shift
  local expect="$1"; shift
  local val
  val=$(curl -s -o /dev/null -D - --max-time 8 "$@" 2>/dev/null | awk -v IGNORECASE=1 -v h="$hdr:" 'tolower($1)==tolower(h){$1=""; sub(/^ +/,""); print; exit}' | tr -d '\r\n')
  if echo "$val" | grep -qiF "$expect"; then
    PASS=$((PASS+1))
    printf "  [PASS] %-60s -> %s: %s\n" "$label" "$hdr" "$val"
  else
    FAIL=$((FAIL+1))
    printf "  [FAIL] %-60s -> %s: '%s' expected to contain '%s'\n" "$label" "$hdr" "$val" "$expect"
  fi
}

# expect_body_contains <label> <expected-substring> <curl-args...>
expect_body_contains() {
  local label="$1"; shift
  local expect="$1"; shift
  local body
  body=$(curl -s --max-time 8 "$@" 2>/dev/null)
  if echo "$body" | grep -qF "$expect"; then
    PASS=$((PASS+1))
    printf "  [PASS] %-60s -> body contains '%s'\n" "$label" "$expect"
  else
    FAIL=$((FAIL+1))
    printf "  [FAIL] %-60s -> body missing '%s' (got %d chars)\n" "$label" "$expect" "${#body}"
  fi
}

# expect_no_crash: assert webserv PID is still alive after a test
assert_alive() {
  local pid="$1"
  if kill -0 "$pid" 2>/dev/null; then
    return 0
  else
    FAIL=$((FAIL+1))
    echo "  [FAIL] webserv (pid $pid) DIED"
    return 1
  fi
}

# raw_request: send literal bytes via nc, return status code from response line.
# usage: raw_request <host> <port> <bytes> [timeout]
raw_request() {
  local host="$1" port="$2" data="$3" tout="${4:-5}"
  printf '%s' "$data" | nc -w "$tout" "$host" "$port" 2>/dev/null
}

# raw_status: extract status from raw response
raw_status() {
  awk 'NR==1 && /^HTTP\// {print $2; exit}'
}

summary() {
  echo ""
  echo "  --- summary: PASS=$PASS  FAIL=$FAIL ---"
}
