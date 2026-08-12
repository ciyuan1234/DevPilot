# 复盘 04：GoogleTest 测试框架接入

> 日期：2026-08-12 ｜ 项目：DevPilot AI（MVP）
> 目标：建立 C++ 单元测试体系，兑现 README"CI 与测试框架"待补项

## 背景

CI 只有 smoke test（"能跑"），没有行为验证（"对不对"）。企业标准下，测试是质量基线：业务代码进入主分支前必须被单测覆盖。这一步同时解决了两个问题：搭测试框架 + 找到第一个真实可测单元。

## 关键决策

### 决策 1：测试对象选"文件名校验"而非"空框架"

后端没有业务代码，直接搭框架会产出"空测试"（形式主义）。从 SDD 第 7/11 节挑了一个纯函数需求：**文件名校验**（文件上传的前置安全校验，防路径穿越/非法后缀）。

理由：
- 纯函数、无 IO 依赖，最适合第一个单测
- 安全敏感（路径穿越是真实漏洞），测试有实战价值
- 是 MVP "文件上传"功能的前置性工作，不浪费

### 决策 2：GoogleTest 用 FetchContent 而非 apt

- `FetchContent` 拉 v1.15.2 源码：版本锁定、CI/本地一致
- apt 的 `libgtest-dev` 版本跟随发行版，不可控
- 代价：CMake 首次配置需网络拉源码（契约已写入 AGENTS.md）

### 决策 3：错误返回用字符串而非 bool

```cpp
std::string validate_source_file_name(const std::string& name);
// "" = 合法；非空 = 错误原因
```

好处：测试断言能区分"为何失败"（空名 vs 路径穿越 vs 后缀非法），后续 API 层可直接把错误原因透传给 HTTP 响应，无需二次映射。

## 测试设计（7 例）

| 分组 | 用例 | 覆盖点 |
|---|---|---|
| 接受 | 常见源码后缀（.cpp/.h/.hpp/.cxx/.c） | 正常路径 |
| 大小写 | Main.CPP / Foo.H | 白名单不区分大小写 |
| 拒绝 | 空/纯空白 | 边界 |
| 拒绝 | a/b.cpp / ..\evil.cpp | 路径穿越（核心安全项） |
| 拒绝 | . / .. | 伪目录 |
| 拒绝 | 256 字符超长名 | 长度上限 |
| 拒绝 | .txt/.sh/.exe/无后缀 | 后缀白名单 |

## 踩坑记录

1. **环境切换**：会话中 shell 从"Windows PowerShell + wsl 包裹"变为"WSL 直连（zsh）"，`wsl` 命令不再存在。已更新 AGENTS.md 环境章节——**这种切换是常态，AGENTS.md 是唯一权威**。
2. **PowerShell 引号转义**：多行内联 python/curl 在 PowerShell 侧反复踩坑，最终统一用"/tmp 写脚本再执行"模式，此经验与 WSL 环境变化一并记入 AGENTS.md。

## 复盘结论

1. **测试先行于业务**：现在有了 ctest 基建 + 7 个真实用例，后续每个业务模块（用户/项目/文件/AI Gateway）的实现都有"测试接缝"可挂。
2. **从 SDD 安全要求出发选测试对象**是低成本高回报的顺序——安全校验函数天生可测、独立、无副作用。
3. **CI 节奏正式确立**：push → 装依赖 → 构建 → ctest → smoke test 全绿才允许合入。失败即红。
4. **遗留事项**：
   - `gh auth login` 仍未配置，CI 日志排查仍靠"本地环境对照法"
   - 文件名校验尚未接入实际上传接口（等文件上传功能落地时接入）
   - 下一个测试目标候选：密码哈希、API Key 校验（SDD 第 11 节）

## 附：当前测试体系

```text
backend/
├── src/util/file_validate.{h,cpp}   # 被测对象（纯函数）
├── tests/test_file_validate.cpp      # 7 个用例
└── CMakeLists.txt                    # FetchContent(googletest v1.15.2) + gtest_discover_tests
CI: ctest --test-dir build --output-on-failure
```