# 09 · 项目管理：多租户归属隔离 + 双实现 repository

> 目标：按 SDD V1 范围落地项目管理。核心价值不在 CRUD 本身，而在**多租户数据隔离**——用户只能看到自己的项目（SDD 第 11 节安全要求）。

## API 范围（与用户确认）

SDD 只定义了 `POST /api/project/create`，但**没有 list 就无法端到端验证归属隔离**，故补充 `GET /api/project/list`（最简）。刻意不做 update/delete——V1 项目一经创建不可改，避免过度设计（YAGNI）。

## 设计落点

### 1. 表结构（schema.sql，幂等可重复执行）

```sql
CREATE TABLE IF NOT EXISTS `project` (
  ...,
  `storage_type` ENUM ('local','object_storage','remote') NOT NULL DEFAULT 'local',
  KEY `idx_user_id` (`user_id`)   -- list_by_user 的查询路径
);
```

- **枚举约束**把非法值挡在数据库层（双层防御：handler 校验 + DB 约束）
- 索引跟着**查询路径**走：list 永远 `WHERE user_id = ?`，建 `idx_user_id`。企业标准：先想清楚查询，再建索引——**索引是为查询服务的，不是越多越好**

### 2. Repository 双实现（模式与前两模块完全一致）

- `IProjectRepository` 抽象：`create_project` + `list_by_user`（协程，MySQL 实现是异步的）
- InMemory：单测与演示；MySQL：参数化查询防注入
- **多租户隔离的关键**：`list_by_user(user_id)` 的 `user_id` **永远来自 token，不从请求体取**。客户端可能谎报 owner，信任 token 是安全红线

### 3. handler 校验（输入契约）

- `name`：必填，1–128 字符（与 DB `VARCHAR(128)` 对齐）
- `storage_type`：不传默认 `local`；显式传必须命中枚举，否则 400
- 校验语义：401（无 token）/ 400（参数错）/ 201（成功）/ 500（仓库失败）

### 4. 测试与验证

- 单测 3 例（共 36 例全绿）：递增 id、**只返回自己的项目**、字段透传 + 新项目在前
- `scripts/verify-project.sh` 9 场景端到端全过，重点：
  - U1 建 2 个、U2 建 1 个
  - U1 list 只见自己的 2 个（newest first），U2 只见 1 个 ✅ 隔离生效
  - 无 token 401、空名 400、非法 storage_type 400

## 企业实践对应

- **多租户隔离**：数据按所有者过滤是 SaaS 安全基础，用集成脚本固化验证
- **双向防御**：应用层校验 + 数据库约束，单一层都可能被绕过
- **最小 API 面**：V1 按需裁剪，扩展留给真实需求（YAGNI）
- **迁移纪律**：schema.sql 幂等，升级脚本可重复执行（生产建议引入版本化迁移工具，如 Flyway/Liquibase——V1 从简）

## 下一步

1. 文件上传（file_validate 工具已就绪，SDD 第 8 节 `POST /api/file/upload`）
2. Agent Metadata + `GET /api/agents`
3. Agent Runtime + CodeExplainAgent + `POST /api/agent/execute`
4. OllamaProvider（等网络）