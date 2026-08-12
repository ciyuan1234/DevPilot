# 复盘 01：Git 仓库初始化与对齐远程仓库

> 日期：2026-08-12 ｜ 项目：DevPilot AI（MVP）
> 目标：按企业标准初始化版本控制，对接 GitHub 远程仓库 `ciyuan1234/DevPilot`

## 背景

项目已有 PRD / SDD 文档和一个 Drogon 后端脚手架，但尚未 `git init`、无 `.gitignore`、README 为空。企业开发的第一步应该是：版本控制先行——所有代码变更都要可追溯、可回滚、可协作。

## 做了什么

| 步骤 | 动作 | 命令（WSL 内） |
|---|---|---|
| 1 | 初始化仓库，默认分支 `main` | `git init -b main` |
| 2 | 配置提交身份（GitHub 隐私邮箱） | `git config user.name ciyuan1234` `git config user.email ciyuan1234@users.noreply.github.com` |
| 3 | 按企业标准补 `.gitignore` | 忽略 build 产物、上传数据、IDE 配置 |
| 4 | 初始提交（7 个文件：文档 + 后端脚手架） | `git add -A && git commit -m "chore: 初始化项目（文档/后端脚手架）"` |
| 5 | 添加远程并推送 | `git remote add origin git@github.com:ciyuan1234/DevPilot.git` `git push -u origin main` |

## 关键决策与理由

### 为什么用 GitHub noreply 邮箱
- 公开仓库中建议使用 `username@users.noreply.github.com`，避免真实邮箱暴露（防垃圾邮件/社工）。
- 企业标准：提交身份应稳定、可归属，且不泄露隐私。

### `.gitignore` 内容（企业标准的"不该提交什么"）
```
backend/build/      # CMake 构建产物
CMakeFiles/         # 根目录误生成的 CMake 临时配置
backend/uploads/    # 运行时文件（上传分片暂存），非源码
.vscode/            # IDE 个人配置（内含本机绝对路径）
```
原则：**源码进库，产物与运行时数据出库**。`backend/uploads/tmp` 的 hash 分片目录就是典型的运行时数据。

## 踩坑记录（重要）

### 坑 1：`git push` 超时 → SSH 公钥认证失败
- 现象：`git push` 卡住直至超时；`ssh -T git@github.com` 报 `Permission denied (publickey)`。
- 原因：WSL 中从未生成 SSH 密钥，GitHub 账号也没添加公钥。
- 解决：
  1. 生成 ed25519 密钥（无需 passphrase，配合 ssh-agent 使用更佳）：
     `ssh-keygen -t ed25519 -C "ciyuan1234@users.noreply.github.com" -f ~/.ssh/id_ed25519 -N ''`
  2. 把 `~/.ssh/id_ed25519.pub` 内容粘贴到 GitHub → Settings → SSH and GPG keys。
  3. 验证：`ssh -T git@github.com` 返回 `Hi ciyuan1234! ...` 即成功。
- 教训：**推送前先验证认证链路**，避免在超时上浪费时间。

### 坑 2：Windows PowerShell 与 WSL 的环境边界
- 主机 shell 是 PowerShell，但代码在 WSL2 里；所有 git/ssh 命令必须 `wsl -e bash -c "..."` 包裹。
- 若直接用 Windows 侧 git 操作 WSL 文件，路径与权限语义会不一致（且 SSH 密钥在 WSL 的 `~/.ssh`）。

## 复盘结论

1. **版本控制先行**是项目启动的第一件企业级事务，后续所有代码变更都应遵循"小步提交、清晰 message"。
2. SSH 密钥是一次性配置，完成后推送链路通畅——`git push -u origin main` 已设置上游跟踪，之后直接 `git push` 即可。
3. 遗留事项：README 仍为空，应在后续步骤补齐（项目简介、开发命令、架构图引用）。
4. 后续建议：配置 CI（GitHub Actions）与分支保护，与"按企业标准完成项目"的目标对齐。

## 附：当前仓库结构

```
docs/PRD-v1.0.md      # 需求权威来源（注意：SDD 文件名带空格）
docs/SDDv- 1.0.md
backend/              # C++20 + Drogon，仅 /api/health
frontend/             # Vue3 规划中（空）
agents/ deploy/       # 空，规划中
AGENTS.md             # 开发协作规范（引导式开发）
```