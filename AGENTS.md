# AGENTS.md

DevPilot AI —— 可扩展的 AI 软件工程平台（MVP 阶段）。需求见 `docs/PRD-v1.0.md`，系统设计见 `docs/SDDv- 1.0.md`（**注意文件名中间有空格**）。两份文档是需求与架构的唯一权威来源，动代码前先对齐它们。

## 用户协作方式（最重要）

用户要求按企业级开发标准完成项目，且**重点在于引导，而非代写**。因此：

- 接到新功能时，先给方案/拆解/取舍并说明理由，让用户确认后再动手；不要一次性把整块功能写完。
- 主动指出企业实践中缺失的环节（测试、异常处理、鉴权、日志、配置管理、迁移、代码评审等），并解释为什么。
- 每一步动手前说明"做什么、为什么、对应什么企业标准"。

## 环境（关键）

- 代码在 WSL2（Ubuntu 24.04）里，但 shell 是 Windows PowerShell。**任何 Linux 命令都要用 `wsl -e bash -c "..."` 包裹**，不能直接调 cmake/make/curl。
- 路径有两种写法：WSL 内 `/home/dev/project/...`；Windows 侧 `\\wsl.localhost\Ubuntu-24.04\home\dev\project\...`。
- 后端依赖已装好：Drogon、jsoncpp、yaml-cpp、mysqlclient、CMake 3.28、g++。

## 后端（backend/，C++20 + Drogon）

- 构建：`wsl -e bash -c "cd /home/dev/project/backend && cmake -S . -B build && cmake --build build"`
- 运行：`wsl -e bash -c "/home/dev/project/backend/build/devpilot-backend"`，监听 `0.0.0.0:8080`
- 验证：`curl http://localhost:8080/api/health`（当前唯一接口，返回 `{"status":"ok"}`，已验证）
- 现状：仅一个 health handler（`backend/src/main.cpp`），其余均为规划。
- `backend/CMakeLists.txt` 硬编码了 MySQL 路径但尚未链接 mysqlclient；真正接 MySQL 时需清理。
- `backend/uploads/tmp` 是文件上传暂存目录（按 hash 分片），非代码，勿提交。

## 架构要点（来自 SDD，务必遵守）

- 平台能力（用户/项目/文件/权限/模型/Agent 管理）与 Agent 能力（PR Review / Debug / Code Explain 等）**解耦**。
- 分层：Client(Vue3+TS+Element Plus) → Platform → Agent Manager → Agent Runtime → AI Gateway → Provider(OpenAI/DeepSeek/Claude/Ollama)。
- AI Gateway 只做模型决策（路由/重试/超时/密钥/Token 计费/上下文拼装），**不做 Agent 调度**；调度归 Agent Runtime。
- Agent 抽象：Name / Version / Description / Input Schema / Output Schema / Tools / Runtime。V1 只做官方 Agent（CodeExplainAgent），**不提供第三方注册接口**。
- 数据库表、HTTP API、安全要求定义在 SDD 第 7/8/11 节（如 API Key 不明文存储，用 `credential_ref` 引用）。
- V1 范围：用户系统、项目管理、文件上传、Agent Metadata、AI Gateway、一个官方 Agent。**不做**：多 Agent 协作、Marketplace、K8s/多节点、第三方 Agent 运行。

## 其他目录

- `frontend/`：Vue3 + TS + Element Plus，尚未创建（无 package.json）。
- `deploy/`：Docker Compose 部署规划，空。
- `agents/`：官方 Agent 实现位置，空。
- `docs/`：PRD 与 SDD（架构权威来源）。

## 待办工程事项

- 尚未 `git init`，无 `.gitignore`（应忽略 `backend/build/`、`backend/uploads/` 等）。
- `README.md` 为空。企业标准下应先初始化仓库、README、CI、测试框架——这是引导用户的合理起点。
