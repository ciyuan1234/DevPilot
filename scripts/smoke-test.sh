#!/usr/bin/env bash
# Smoke test: start backend, assert health endpoint returns {"status":"ok"}
set -euo pipefail

cd "$(dirname "$0")/.."

./backend/build/devpilot-backend >/tmp/dp.log 2>&1 &
PID=$!
trap 'kill $PID 2>/dev/null || true' EXIT

for i in $(seq 1 10); do
  if curl -sf http://localhost:8080/api/health >/dev/null 2>&1; then
    break
  fi
  sleep 1
done

BODY=$(curl -s http://localhost:8080/api/health)
CODE=$(curl -s -o /dev/null -w '%{http_code}' http://localhost:8080/api/health)
echo "health response: HTTP ${CODE} ${BODY}"
test "$BODY" = '{"status":"ok"}' && test "$CODE" = "200"
echo "SMOKE TEST PASSED"
