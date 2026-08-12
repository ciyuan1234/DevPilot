# 复盘 03：GitHub Actions CI 搭建

> 日期：2026-08-12 ｜ 项目：DevPilot AI（MVP）
> 目标：建立"提交即验证"的持续集成，补齐企业标准的第一块拼图

## 背景

README 已就绪，但没有任何自动化验证。企业标准下"代码合入前必须经过 CI 校验"，并且 README 里已留着 CI 徽章和"CI 与测试框架待配置"的坑位——这一步兑现承诺。

## 做了什么

| 内容 | 说明 |
|---|---|
| `.github/workflows/ci.yml` | push / PR 触发；5 步：checkout → 装依赖 → configure → build → smoke test |
| `scripts/smoke-test.sh` | 启动后端 → 轮询 health → 断言 `{"status":"ok"}` + HTTP 200，失败即红 |
| README CI 徽章 | `badge.svg` 链接，显示真实运行状态 |

### 关键设计：smoke test 单一来源

最初把 curl 逻辑内联在 workflow 里，后改为独立脚本 `scripts/smoke-test.sh`，CI 仅一行 `bash scripts/smoke-test.sh`。理由：

- 本地与 CI 行为一致（本地先验证通过再提交，CI 就不会因"环境差异"红）
- 逻辑只维护一份——企业标准的 DRY 原则
- 后续加测试时，这个脚本就是"集成测试"的雏形

## 踩坑记录（CI 两连败，价值最大）

### 坑 1：`libdrogon-dev` 的依赖 ≠ 构建所需依赖

- 现象：configure 步骤失败。
- 根因：`libdrogon-dev` 在 Ubuntu 24.04 只自动依赖 `libdrogon1t64` + `libtrantor-dev`，但它的 `DrogonConfig.cmake` 通过 `find_dependency()` 隐式 REQUIRED 查找 MySQL / PostgreSQL / Boost / Brotli / SQLite / yaml-cpp 等十多个开发包——**这些都不是 apt 自动依赖**。
- 排查法：`grep find_dependency /usr/lib/x86_64-linux-gnu/cmake/Drogon/DrogonConfig.cmake`，对照本机 `dpkg -l` 逐项补齐（本机环境 = GitHub runner 同为 Ubuntu 24.04，可以照抄）。
- 教训：**第三方库的 CMake 打包依赖不看 apt depends，要看它的 Config.cmake**。

### 坑 2：yaml-cpp 漏装

- configure 仍失败后，对照 DrogonConfig.cmake 的 find_dependency 列表逐项核对，发现本机独有 `libyaml-cpp-dev`。补装后通过。
- 教训：排查依赖差异要"逐项对照"，而不是猜。

### 坑 3：不看日志瞎猜

- 前两次失败都只拿到 `exit code 1`（匿名 API 无法读 Actions 日志，需认证）。
- 最终靠"本地环境对照 + 逐个 apt 包比对"定位，没走认证流程。
- 效率提示：若配置 `gh auth login`，日志一步到位，不用猜——此教训记入博客供日后复用。

## 复盘结论

1. **CI 的核心价值是"失败即红"**：configure 失败就停止，不让错误代码合并。
2. **本地 = CI 环境对照法有效**：同为 Ubuntu 24.04，依赖清单可以"照抄本机"，大幅降低试错成本。
3. **smoke test 脚本化**是本步最重要的工程决策，为后续接入 GoogleTest 单元测试留了接缝。
4. **遗留事项**：
   - 后宫测试框架（GoogleTest + CTest）未装，CI 里只有 smoke test
   - `gh auth login` 未完成，后续查 CI 日志仍会受阻
   - README 徽章已亮绿灯，可截图存档

## 附：当前 CI 全貌

```yaml
on: [push, pull_request]
jobs:
  backend-build:
    runs-on: ubuntu-latest
    steps: checkout -> install deps -> cmake configure -> cmake build -> smoke-test.sh
```