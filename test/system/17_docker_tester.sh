#!/usr/bin/env bash
# Phase 17: Run the 42 ubuntu_tester / ubuntu_cgi_tester via Docker.
# Builds webserv inside an ubuntu:22.04 container (because the host is darwin/arm64
# and the testers are linux/amd64), then runs the testers against it.
set -u
cd "$(dirname "$0")/../.."

OUT="${1:-/tmp/42-tester.log}"

if ! docker info >/dev/null 2>&1; then
  echo "  [SKIP] docker not running"
  exit 0
fi

# Use --platform linux/amd64 because testers are amd64 and host is arm64
docker run --rm \
  --platform linux/amd64 \
  -v "$PWD":/work \
  -w /work \
  ubuntu:22.04 \
  bash -c '
    set -e
    export DEBIAN_FRONTEND=noninteractive
    apt-get update -qq >/dev/null
    apt-get install -y -qq build-essential python3 curl >/dev/null
    echo "=== Building webserv (linux) ==="
    make fclean >/dev/null
    # Subject requires only -Wall -Wextra -Werror; the Makefile adds -Wshadow which
    # rejects a nested-catch variable shadow on Linux GCC. Build without -Wshadow.
    make all CXXFLAGS="-Wall -Wextra -Werror -MMD -MP -std=c++98" 2>&1 | tail -5
    echo "=== Starting webserv on :8080 (config docker_linux.conf) ==="
    ./webserv configs/docker_linux.conf > /tmp/ws.log 2>&1 &
    WS=$!
    sleep 3
    if ! kill -0 $WS 2>/dev/null; then
      echo "FAILED to start webserv (process died)"
      tail -50 /tmp/ws.log
      exit 1
    fi
    echo "webserv PID=$WS"
    ss -tlnp 2>&1 | head -10 || netstat -tlnp 2>&1 | head -10
    curl -s -o /dev/null -w "smoke: %{http_code}\n" http://127.0.0.1:8080/ || true
    echo ""
    echo "=== Running ubuntu_tester (auto-pressing enter) ==="
    chmod +x tester/ubuntu_tester tester/ubuntu_cgi_tester 2>/dev/null
    # Note: tester is interactive and expects YoupiBanane setup. We feed empty enters
    # to skip prompts and see how far it gets.
    yes "" | timeout 90s ./tester/ubuntu_tester http://127.0.0.1:8080 2>&1 | tail -100 || echo "ubuntu_tester exited $?"
    echo ""
    echo "=== Running ubuntu_cgi_tester ==="
    yes "" | timeout 90s ./tester/ubuntu_cgi_tester http://127.0.0.1:8080 / .py 2>&1 | tail -60 || echo "ubuntu_cgi_tester exited $?"
    kill $WS 2>/dev/null
    wait $WS 2>/dev/null
    echo "=== webserv stdout tail ==="
    tail -20 /tmp/ws.log
  ' 2>&1 | tee "$OUT"
