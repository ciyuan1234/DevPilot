# DevPilot AI

> DevPilot AI —— 面向软件工程生命周期的 AI 平台，通过插件化 Agent 架构扩展 AI 能力。

当前处于 **MVP 阶段**：后端脚手架已就绪（C++20 + Drogon），提供健康检查接口；前端与 Agent 能力建设中。

## 核心特性

- **项目管理**：创建项目、上传文件，为 AI 分析提供输入
- **AI Gateway**：统一接入 OpenAI / DeepSeek / Claude / Ollama，隔离模型实现
- **Agent 架构**：平台能力与 Agent 能力解耦，首个官方 Agent 为 CodeExplainAgent

## 文档

- [产品需求文档（PRD）](docs/PRD-v1.0.md)
- [系统设计文档（SDD）](docs/SDDv-%201.0.md)