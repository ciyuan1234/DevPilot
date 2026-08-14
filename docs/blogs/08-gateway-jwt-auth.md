# 08 · Gateway API 接入 JWT 鉴权：显式守卫 vs 中间件

> 目标：平台 API 必须鉴权（SDD 第 11 节安全要求）。为 `/api/gateway/*` 接入 Bearer Token 校验，补齐用户系统闭环：**注册 → 登录拿 token → 带 token 调业务 API**。

## 设计取舍：守卫函数，不用中间件

Drogon 官方提供中间件（HttpMiddleware）做横切鉴权，但本阶段我选择**显式守卫函数**：

| 方案 | 优点 | 代价 |
|---|---|---|
| Drogon 中间件 | 横切、声明式、适合几十个接口 | 本版本注册/过滤名契约繁琐且跨版本易碎；多一层间接，调试要翻框架代码 |
| 守卫函数 `require_auth(req, secret)` | 显式、可单测、一行看懂 | 每个受保护 handler 里多一行调用 |

选择依据（YAGNI）：V1 受保护接口只有 2 个，中间件带来的收益不抵复杂度。**当受保护接口数量上规模时再重构成中间件**——架构要留得出重构空间（守卫函数是纯函数，重构成本极低）。

## 实现

### 1. `auth/auth_guard.{h,cpp}`

```cpp
// 从 Authorization: Bearer <token> 提取并校验 JWT
std::optional<uint64_t> require_auth(const HttpRequestPtr& req, const std::string& jwt_secret);
```

- 只认 `Bearer ` 前缀（`std::string_view` 比较，零拷贝）
- 成功返回 `user_id`（业务后续可用）；失败返回 nullopt，**由 handler 决定响应**（401）
- 校验委托给已有的 `verify_jwt`（jwt-cpp，HS256，检查过期/发行者）

### 2. 接入 handler

- `handle_chat` / `handle_providers` 各加参数 `jwt_secret`，开头一行守卫，失败回 `401 {"error":"unauthorized"}`
- 守卫与业务解耦：handler 只关心"过了没有"，不关心"怎么验"

### 3. 单测（6 例，纯函数、毫秒级）

缺头 / 错误 scheme（Basic）/ 垃圾 token / 有效 token（返回正确 uid）/ 错误密钥 / **过期 token**（`ttl=-1s` 签发即过期）。全量 33 测全绿。

### 4. 端到端脚本 `scripts/verify-gateway.sh`

7 场景：注册 201 → 登录取 token → chat 无 token 401 / 错 token 401 / 带 token 200 → providers 无 token 401 / 带 token 200。全过。

## 企业实践对应

- **零信任**：业务接口默认要凭据，401 语义统一
- **安全日志**：守卫失败未打日志（V1 从简），生产应记录来源 IP + 原因并防暴力枚举
- **自动化验证**：鉴权场景进脚本化冒烟（scripts/），回归成本为零
- **单测优先**：鉴权逻辑是纯函数，先测死契约再谈集成

## 遗留

- 注册/登录本身无 Rate Limit（防爆破）；`DEVPILOT_JWT_SECRET` 仍建议通过环境变量注入
- chat 响应里未带 user_id（守卫返回值暂未用，等 Agent Runtime 需要用户上下文时启用）