#!/usr/bin/env bash
# 一键验证：Gateway API + JWT 鉴权（起 MySQL + 后端 → 注册/登录取 token → 验证 401/200 → 清理）
set -euo pipefail

echo "=== starting mysql ==="
sudo service mysql start
sleep 3

echo "=== starting backend ==="
cd /home/dev/project/backend
./build/devpilot-backend > /tmp/backend.log 2>&1 &
PID=$!
trap 'kill $PID 2>/dev/null || true' EXIT
sleep 4

BASE=http://localhost:8080
USER="gw$(date +%s)"

echo "=== 1. register $USER ==="
curl -s -w '\nHTTP:%{http_code}\n' -X POST $BASE/api/auth/register \
  -H "Content-Type: application/json" -d "{\"username\":\"$USER\",\"password\":\"gw123456\"}"

echo "=== 2. login, extract token ==="
TOKEN=$(curl -s -X POST $BASE/api/auth/login \
  -H "Content-Type: application/json" -d "{\"username\":\"$USER\",\"password\":\"gw123456\"}" \
  | python3 -c 'import sys,json; print(json.load(sys.stdin)["token"])')
echo "token: ${TOKEN:0:24}..."

echo "=== 3. chat WITHOUT token (expect 401) ==="
curl -s -w '\nHTTP:%{http_code}\n' -X POST $BASE/api/gateway/chat \
  -H "Content-Type: application/json" -d '{"provider":"fake","messages":[{"role":"user","content":"hi"}]}'

echo "=== 4. chat with WRONG token (expect 401) ==="
curl -s -w '\nHTTP:%{http_code}\n' -X POST $BASE/api/gateway/chat \
  -H "Content-Type: application/json" -H "Authorization: Bearer garbage.token.here" \
  -d '{"provider":"fake","messages":[{"role":"user","content":"hi"}]}'

echo "=== 5. chat WITH token (expect 200 + fake reply) ==="
curl -s -w '\nHTTP:%{http_code}\n' -X POST $BASE/api/gateway/chat \
  -H "Content-Type: application/json" -H "Authorization: Bearer $TOKEN" \
  -d '{"provider":"fake","model":"m1","messages":[{"role":"user","content":"hello auth"}]}'

echo "=== 6. providers WITHOUT token (expect 401) ==="
curl -s -w '\nHTTP:%{http_code}\n' $BASE/api/gateway/providers

echo "=== 7. providers WITH token (expect 200 [\"fake\"]) ==="
curl -s -w '\nHTTP:%{http_code}\n' -H "Authorization: Bearer $TOKEN" $BASE/api/gateway/providers

echo "=== done ==="
