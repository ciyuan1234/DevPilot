# 复盘 06：MySQL 持久化接入

> 日期：2026-08-14 ｜ 项目：DevPilot AI（MVP）
> 目标：用户存储从内存换成 MySQL，数据落盘不丢失

## 做了什么

| 内容 | 说明 |
|---|---|
| `MysqlUserRepository` | IUserRepository 的 MySQL 实现，参数化查询防注入 |
| `config.example.json` | 数据库连接模板（占位符），config.json 不入库 |
| main.cpp | `registerBeginningAdvice` 中初始化 DbClient |
| `scripts/verify-mysql.sh` | 一键拉起环境 + 注册/登录/查库/查重验证 |
| `scripts/verify-persist.sh` | 重启后端后验证数据仍在（持久化核心证据） |

## 关键教学点

1. **抽象的价值实证**：`IUserRepository` 接口让 MySQL 实现替换内存实现时，handler 层**零改动**——分层设计的第一次实战回报。
2. **配置与代码分离**：数据库密码绝不入库，config.json gitignore，仓库只存占位符模板。
3. **SQL 注入防御**：全部用 `?` 占位符参数化查询，绝不字符串拼接用户输入。
4. **接口设计教训**：最初 `IUserRepository` 设计成同步返回，MySQL 异步实现逼着接口改成协程（Task）。**抽象设计时要考虑"最慢实现的形态"**——这个教训值钱。

## 踩坑记录（三连坑，均已在代码层修复）

### 坑 1：`getDbClient` 必须在 run() 之后 —— 空壳 DbClient 段错误
- 现象：gdb 抓到 `drogon::orm::DbClient::operator<<` SIGSEGV。
- 根因：Drogon 契约——`getDbClient` 必须在框架 run 之后调用，之前拿到的是未初始化的空壳客户端。
- 解决：移到 `registerBeginningAdvice` 回调（框架启动前一刻执行）。
- 排查工具：gdb。**崩溃必抓栈**是企业调试第一原则，此例 gdb 一次定位。

### 坑 2：execSqlCoro 返回 SqlAwaiter，必须 co_await
- 现象：编译错误 `SqlAwaiter has no member named 'affectedRows'`。
- 根因：`execSqlCoro` 返回可等待对象（awaiter），不是结果；要 `co_await` 才得 Result。
- 收获：异步编程的核心认知——拿到 awaiter ≠ 拿到结果。
- 连锁影响：仓库方法、接口、handler、测试全部升级为协程语法（`co_return`/`co_await`/`sync_wait`）。

### 坑 3：CLI 会话隔离，MySQL/后端无法常驻
- 现象：curl 请求挂起（后端等 MySQL 无响应），MySQL 进程反复消失。
- 根因：本 CLI 环境每次 bash 调用隔离会话，`service mysql start` 起的守护进程不会常驻；后端进程同理。
- 解决：验证统一走 `scripts/` 脚本——单个 bash 调用内"起 MySQL→起后端→curl→清理"，一次跑完。
- 教训：**环境特性要文档化**（AGENTS.md），避免每次重复踩。

## 验证结果

```
重启验证（verify-persist.sh）：
  carol 注册 → 201；登录 → 200 + JWT；重复注册 → 409（唯一索引拦截）
  重启后端 → carol 仍可登录（数据在 MySQL）；frank 注册成功
```

## 复盘结论

1. **持久化是"demo"与"系统"的分水岭**：内存版重启丢数据，MySQL 版重启数据仍在。
2. **数据库异常要优雅处理**：唯一索引冲突（1062）被捕获转为 409——异常不是 bug，是业务信号。
3. **遗留事项**：
   - `DEVPILOT_JWT_SECRET` 正式密钥管理（现在还是开发默认值）
   - 登录 Rate Limit（防爆破）
   - 连接池参数调优（现在固定 4 连接）
   - CI 里 MySQL 集成测试（当前 CI 只在 Ubuntu runner 跑，无 MySQL）