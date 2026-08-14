#!/usr/bin/env bash
# 一键验证：项目管理 API（鉴权 + 创建 + 列表 + 多用户归属隔离）
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
U1="p$(date +%s)a"
U2="p$(date +%s)b"

login() { # $1=username
  curl -s -X POST $BASE/api/auth/login -H "Content-Type: application/json" \
    -d "{\"username\":\"$1\",\"password\":\"proj123456\"}" \
    | python3 -c 'import sys,json; print(json.load(sys.stdin)["token"])'
}

echo "=== 1. register two users ==="
curl -s -w ' [%{http_code}]\n' -o /dev/null -X POST $BASE/api/auth/register \
  -H "Content-Type: application/json" -d "{\"username\":\"$U1\",\"password\":\"proj123456\"}"
curl -s -w ' [%{http_code}]\n' -o /dev/null -X POST $BASE/api/auth/register \
  -H "Content-Type: application/json" -d "{\"username\":\"$U2\",\"password\":\"proj123456\"}"

T1=$(login "$U1")
T2=$(login "$U2")

echo "=== 2. create WITHOUT token (expect 401) ==="
curl -s -w '\nHTTP:%{http_code}\n' -X POST $BASE/api/project/create \
  -H "Content-Type: application/json" -d '{"name":"noauth"}'

echo "=== 3. create with empty name (expect 400) ==="
curl -s -w '\nHTTP:%{http_code}\n' -X POST $BASE/api/project/create \
  -H "Content-Type: application/json" -H "Authorization: Bearer $T1" -d '{"name":""}'

echo "=== 4. create with bad storage_type (expect 400) ==="
curl -s -w '\nHTTP:%{http_code}\n' -X POST $BASE/api/project/create \
  -H "Content-Type: application/json" -H "Authorization: Bearer $T1" \
  -d '{"name":"bad","storage_type":"galactica"}'

echo "=== 5. U1 creates two projects (expect 201) ==="
curl -s -w ' [%{http_code}]\n' -X POST $BASE/api/project/create \
  -H "Content-Type: application/json" -H "Authorization: Bearer $T1" \
  -d '{"name":"web-app","language":"typescript"}'
curl -s -w ' [%{http_code}]\n' -X POST $BASE/api/project/create \
  -H "Content-Type: application/json" -H "Authorization: Bearer $T1" \
  -d '{"name":"cli-tool","storage_type":"local","storage_reference":"/data/cli","language":"go"}'

echo "=== 6. U2 creates one project ==="
curl -s -w ' [%{http_code}]\n' -X POST $BASE/api/project/create \
  -H "Content-Type: application/json" -H "Authorization: Bearer $T2" \
  -d '{"name":"other-app"}'

echo "=== 7. U1 list (expect 2, newest first) ==="
curl -s -H "Authorization: Bearer $T1" $BASE/api/project/list | python3 -m json.tool
echo "=== 8. U2 list (expect 1, isolation) ==="
curl -s -H "Authorization: Bearer $T2" $BASE/api/project/list | python3 -m json.tool

echo "=== 9. db rows ==="
mysql -h 127.0.0.1 -P 3306 -u devpilot -pdevpilot-dev devpilot \
  -e "SELECT id, user_id, name, storage_type, storage_reference, language FROM project ORDER BY id;" 2>/dev/null

echo "=== done ==="