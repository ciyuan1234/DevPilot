#!/usr/bin/env bash
# 一键开发环境：拉起 MySQL + 后端，跑持久化冒烟验证后清理
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

echo "=== 1. register carol ==="
curl -s -w '\nHTTP:%{http_code}\n' -X POST http://localhost:8080/api/auth/register \
  -H "Content-Type: application/json" -d '{"username":"carol","password":"persist123"}'

echo "=== 2. login carol ==="
curl -s -w '\nHTTP:%{http_code}\n' -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" -d '{"username":"carol","password":"persist123"}'

echo "=== 3. db rows ==="
mysql -h 127.0.0.1 -P 3306 -u devpilot -pdevpilot-dev devpilot \
  -e "SELECT id, username, LEFT(password_hash,35) AS hash_prefix, create_time FROM user;" 2>/dev/null

echo "=== 4. duplicate register (expect 409) ==="
curl -s -w '\nHTTP:%{http_code}\n' -X POST http://localhost:8080/api/auth/register \
  -H "Content-Type: application/json" -d '{"username":"carol","password":"persist123"}'

echo "=== backend log ==="
cat /tmp/backend.log || true
