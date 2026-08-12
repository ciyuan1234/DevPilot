# DevPilot AI

> DevPilot AI —— 面向软件工程生命周期的 AI 平台，通过插件化 Agent 架构扩展 AI 能力。

当前处于 **MVP 阶段**：后端脚手架已就绪（C++20 + Drogon），提供健康检查接口；前端与 Agent 能力建设中。

## 核心特性

- **项目管理**：创建项目、上传文件，为 AI 分析提供输入
- **AI Gateway**：统一接入 OpenAI / DeepSeek / Claude / Ollama，隔离模型实现
- **Agent 架构**：平台能力与 Agent 能力解耦，首个官方 Agent 为 CodeExplainAgent

## 项目结构

```text
DevPilot/
├── backend/     # C++20 + Drogon 后端（CMake 构建，产物在 backend/build/）
├── frontend/    # Vue3 + TS + Element Plus（规划中，暂无代码）
├── agents/      # 官方 Agent 实现（首个为 CodeExplainAgent）
├── deploy/      # Docker Compose 部署（规划中）
└── docs/
    ├── PRD-v1.0.md     # 产品需求文档
    ├── SDDv- 1.0.md    # 系统设计文档
    └── blogs/          # 开发复盘（按步骤编号，如 01-git-init-and-remote.md）
```

## 快速开始

前置依赖：CMake ≥ 3.20、g++（支持 C++20）、Drogon 框架。

```bash
cd backend
cmake -S . -B build
cmake --build build
```

运行服务（监听 `0.0.0.0:8080`）：

```bash
./build/devpilot-backend
```

验证（另开终端）：

```bash
curl http://localhost:8080/api/health
# 期望输出：{"status":"ok"}
```

## 技术栈

| 层 | 技术 |
|---|---|
| 前端 | Vue3 + TypeScript + Element Plus |
| 后端 | C++20 + Drogon |
| 数据库 | MySQL + Redis |
| 部署 | Docker Compose |
| 模型接入 | OpenAI / DeepSeek / Claude / Ollama |

## 架构

平台能力与 Agent 能力**解耦**：平台负责用户、项目、文件、权限与模型管理；Agent 负责具体软件工程任务（PR Review / Debug / Code Explain 等）。

```text
Client (Vue3 + TS + Element Plus)
   │
Platform (用户 / 项目 / 文件 / 权限)
   │
Agent Manager (注册 / 配置 / 版本)
   │
Agent Runtime (创建实例 / 调度 / 资源限制)
   │
AI Gateway (模型选择 / 密钥管理 / Token 计费)
   │
Provider (OpenAI / DeepSeek / Claude / Ollama)
```

> AI Gateway 只负责模型决策，不做 Agent 调度；调度归 Agent Runtime。详细设计见 [SDD](docs/SDDv-%201.0.md)。

## 路线图与边界

V1（当前）按序推进：用户系统 → 项目管理 → 文件上传 → Agent Metadata → AI Gateway → 官方 Agent（CodeExplainAgent）。

V1 明确**不做**：多 Agent 协作、Agent Marketplace、K8s / 多节点部署、第三方 Agent 运行。

## 开发约定

- **引导式开发**：功能开发先给方案与取舍，确认后再动手（见 AGENTS.md）
- **复盘归档**：每完成一步，在 `docs/blogs/` 按编号写复盘博客
- **CI 与测试框架**：尚未配置（企业标准待补项）

## 文档

| 文档 | 说明 |
|---|---|
| [产品需求文档（PRD）](docs/PRD-v1.0.md) | 项目定位、MVP 功能范围、成功指标 |
| [系统设计文档（SDD）](docs/SDDv-%201.0.md) | 分层架构、数据库设计、API 定义、安全设计 |
| [开发复盘博客](docs/blogs/) | 按步骤编号的开发记录，用于复盘与知识沉淀 |
| [AGENTS.md](AGENTS.md) | 开发协作规范（引导式开发 + 环境约定） |