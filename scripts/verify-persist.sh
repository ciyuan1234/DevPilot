#!/usr/bin/env bash
# 验证持久化：重启后端后，之前注册的 carol 仍可登录（数据来自 MySQL 而非内存）
set -euo pipefail

cd /home/dev/project/backend
./build/devpilot-backend > /tmp/backend.log 2>&1 &
PID=$!
trap 'kill $PID 2>/dev/null || true' EXIT
sleep 4

echo "=== restart, then login carol (expect 200) ==="
curl -s -w '\nHTTP:%{http_code}\n' -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" -d '{"username":"carol","password":"persist123"}'

echo "=== wrong password (expect 401) ==="
curl -s -w '\nHTTP:%{http_code}\n' -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" -d '{"username":"carol","password":"wrong"}'

echo "=== register new user after restart (expect 201) ==="
curl -s -w '\nHTTP:%{http_code}\n' -X POST http://localhost:8080/api/auth/register \
  -H "Content-Type: application/json" -d '{"username":"frank","password":"persist456"}'

echo "=== all users in db ==="
mysql -h 127.0.0.1 -P 3306 -u devpilot -pdevpilot-dev devpilot \
  -e "SELECT id, username FROM user ORDER BY id;" 2>/dev/null
