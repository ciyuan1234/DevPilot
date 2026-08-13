# 复盘 05：用户系统 V1（注册/登录 + JWT 认证）

> 日期：2026-08-12 ｜ 项目：DevPilot AI（MVP）
> 目标：MVP 闭环第一块业务——用户系统，对接 SDD 第 7/8 节

## 做了什么

| 模块 | 文件 | 说明 |
|---|---|---|
| 密码哈希 | `src/util/password_hash.{h,cpp}` | argon2id（OWASP 推荐），PHC 格式，含参数自描述 |
| 存储抽象 | `src/auth/user_repository.{h,cpp}` | `IUserRepository` 接口 + InMemory 实现 |
| JWT 工具 | `src/auth/jwt.{h,cpp}` | HS256 签发/校验，含过期时间 |
| HTTP handler | `src/auth/auth_handlers.{h,cpp}` | 注册/登录协程 handler |
| 主程序 | `src/main.cpp` | 路由注册 + 密钥管理 |
| 建表 | `db/schema.sql` | User 表（对齐 SDD 第 7 节） |

### 教学向决策

1. **密码强度校验**：≥8 位含字母数字（V1 基础策略），注册时拒绝弱密码。
2. **防用户名枚举**：登录失败统一返回 401 `invalid credentials`，不区分"用户不存在/密码错误"。
3. **密钥管理起步**：`DEVPILOT_JWT_SECRET` 环境变量，未设置时用开发默认值并 LOG_WARN（配置与代码分离的第一步）。
4. **HTTP 状态码语义**：201 创建 / 409 冲突 / 400 参数非法 / 401 未认证 / 200 成功。

## 踩坑记录（协程 handler 三连坑，血泪教训）

### 坑 1：lambda 包装协程 handler 不被 FunctionTraits 识别

- 错误：`FunctionTraits<lambda(const HttpRequestPtr&)>::first_param_type` 不存在
- 原因：`drogon::HttpBinder` 用 `FunctionTraits` 在**编译期**推导 handler 签名；带捕获的 lambda（`std::function` / `std::bind` / 捕获 `[user_repo]`）都走不通——Traits 只特化了"函数指针"和"无捕获 lambda"路径。
- 解决：改用**自由函数**（无捕获），通过全局指针传依赖。

### 坑 2：参数必须是按值 `HttpRequestPtr`

- 错误：`Task<HttpResponsePtr> (*)(const HttpRequestPtr&)` 不匹配任何特化
- 原因：FunctionTraits 只特化了 `Task<HttpResponsePtr>(HttpRequestPtr)`（**按值**）。`const&` 签名完全匹配不上。
- 解决：`do_register(drogon::HttpRequestPtr req)` 按值传参。

### 坑 3：`drogon::Task` 头文件缺失

- 错误：`'Task' in namespace 'drogon' does not name a template type`
- 原因：`<drogon/drogon.h>` 不包含协程定义，`Task` 在 `<drogon/utils/coroutine.h>`。
- 解决：显式 `#include <drogon/utils/coroutine.h>`。

### 坑 4：主程序 target 缺 include 路径

- 错误：`util/file_validate.h: No such file or directory`
- 原因：测试 target 配了 `target_include_directories(PRIVATE src)`，主程序没配。
- 解决：给 `devpilot-backend` 同样配置。

## 实测验证（5 场景）

| # | 场景 | 预期 | 实测 |
|---|---|---|---|
| 1 | 注册 alice | 201 + user_id | ✅ |
| 2 | 重复注册 | 409 | ✅ |
| 3 | 弱密码 | 400 | ✅ |
| 4 | 登录成功 | 200 + JWT | ✅ |
| 5 | 密码错误 | 401 | ✅ |

教学点：第 4 步返回的 JWT 可以 base64 解码第 2 段亲眼看到 payload 内容。

## 复盘结论

1. **Drogon 协程 handler 的签名是硬约束**：`Task<HttpResponsePtr>(HttpRequestPtr)` 按值 + 自由函数。这个坑一次踩透，后续所有协程 handler 直接照模板写。
2. **抽象的价值验证**：`IUserRepository` 接口让"内存版 → MySQL 版"切换只改一行注入，业务代码零改动——这是分层架构意义的第一次实证。
3. **测试与实测结合**：23 单测管"函数正确"，5 场景 curl 管"链路正确"，互补不替代。
4. **遗留事项**：
   - InMemory 存储重启丢失 → 下一步接 MySQL 持久化
   - `DEVPILOT_JWT_SECRET` 正式配置化（配置文件/密钥管理）
   - 密码强度策略可升级（正则、常用密码黑名单）
   - 登录接口尚未接 Rate Limit（防爆破，V2 规划）

## 附：协程 handler 标准模板（照抄）

```cpp
// 头文件
drogon::Task<drogon::HttpResponsePtr> handle_xxx(drogon::HttpRequestPtr req, /*依赖*/);

// 自由函数（main.cpp）
drogon::Task<drogon::HttpResponsePtr> do_xxx(drogon::HttpRequestPtr req) {
    return devpilot::auth::handle_xxx(req, g_dep);
}
// 注册
drogon::app().registerHandler("/api/xxx", &do_xxx);
```
