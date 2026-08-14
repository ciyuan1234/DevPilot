# 07 · AI Gateway 架构先行：接口抽象与 Mock 实现

> 目标：按 SDD 第 4.3 节落地 AI Gateway。网络受限装不了 Ollama，改用"接口先行 + Mock 实现"完成架构验证——这是企业实践的标准动作：**先验证架构正确性，再填真实实现**。

## 背景：网络受限，Ollama 装不上

尝试装 Ollama 的经过：

| 方式 | 结果 |
|---|---|
| `curl -fsSL https://ollama.com/install.sh \| sh` | 300s 超时 |
| 直接下载 `ollama-linux-amd64.tgz` | **404**：新版本改用了 `.tar.zst` 格式（1.4GB） |
| GitHub Releases 下载 `.tar.zst` | 15 分钟只下了 3.8MB / 1.4GB |

教训：**大二进制下载对网络质量敏感；做项目要有"网络不可用时的降级路径"**。与用户商议后决定：先用 FakeProvider（内存 Mock）完成 Gateway 架构与测试，Ollama 实现之后按同一接口补齐即插即用。

## 分层落地（严格按 SDD）

### 1. 抽象接口 `IProvider`（gateway/provider.h）

```cpp
class IProvider {
public:
    virtual ~IProvider() = default;
    virtual std::string name() const = 0;      // 注册名
    virtual ChatResult complete(const ChatRequest&) = 0;  // 对话补全
};
```

- `ChatRequest`：`model` + `messages[]`（role/content）
- `ChatResult`：`content` + token 统计（prompt/completion）
- **与具体供应商无关**：OpenAI/DeepSeek/Ollama 都只是实现细节

### 2. 路由核心 `Gateway`（gateway/gateway.cpp）

- `register_provider()` 注册（同名覆盖——"最后一次生效"语义）
- `complete(provider, req)` 按名查表转发，未注册抛 `std::out_of_range`
- `providers()` 返回能力列表（供 API 对外查询）
- **边界清晰**：Gateway 只做模型决策/转发，**不做 Agent 调度**（那是 Agent Runtime 的职责，SDD 明确划分）

### 3. FakeProvider（内存 Mock）

固定回复 + 统计 prompt token（按内容长度估算）。单测可确定性断言，完全无网络依赖。

### 4. HTTP 层（步骤 D）

- `POST /api/gateway/chat`：解析 `{provider, model, messages}`，转发，返回结果
- `GET /api/gateway/providers`：返回已注册 provider 列表
- 错误语义：400（缺 provider/空 messages）、404（未知 provider）、502（供应商调用失败）

### 5. 依赖替换的演示

main.cpp 的 `registerBeginningAdvice` 里注册 Provider，之后把 `FakeProvider` 换成 `OllamaProvider` 只需改一行。**这就是"面向接口编程"的好处**：架构不受具体实现影响，实现可独立演进。

## 踩坑 1：CMake 的 CURL 目标不存在

直接写 `target_link_libraries(... CURL::libcurl)` 报错。原因：**没有 `find_package(CURL REQUIRED)` 时 target 不存在**。CMake 约定：`::` 形式的目标必须先经 find_package 找到模块，再在 `target_link_libraries` 里引用。

## 踩坑 2：本版 Drogon 的 handler 签名契约（重要）

想注册一个"同步返回式" handler `HttpResponsePtr(const HttpRequestPtr&)`，编译报：

```
no type named 'first_param_type' in FunctionTraits<...>
```

读 `/usr/include/drogon/utils/FunctionTraits.h` 后确认：**本版 Drogon（1.8.x apt）只支持三种 HTTP handler 签名**：

| 形式 | 签名 | 场景 |
|---|---|---|
| 同步回调式 | `void(const HttpRequestPtr&, std::function<void(const HttpResponsePtr&)>&&, ...)` | 同步逻辑（本项目 `/api/health`、`/providers`） |
| 协程返回式 | `Task<HttpResponsePtr>(HttpRequestPtr)` | 协程逻辑（register/login/chat） |
| 协程回调式 | `Task<>(HttpRequestPtr, callback)` | 少见 |

纯同步返回式**不支持**，会被 FunctionTraits 当成普通函数导致 `registerHandler` 编译失败。教训：**跨版本 Drogon 的 handler API 差异大，动手前先读本机头文件的特化清单**。

## 测试与验证

- 单测 4 例（test_gateway.cpp）：路由转发、未知 provider 抛异常、能力列表、同名覆盖
- 全量 27 测全绿（原 23 + 新 4）
- 端到端 3 场景全通：
  - `GET /api/gateway/providers` → `["fake"]`
  - `POST /api/gateway/chat` → 200 + fake 回复 + token 计数
  - 未知 provider → `{"error":"unknown provider: openai"}`

## 企业实践对应

- **接口先行（contract-first）**：架构正确性先于实现
- **依赖倒置**：Gateway 依赖 `IProvider` 抽象，不依赖任何具体供应商
- **可测试性**：Mock 让单测快、确定、无网络
- **变更同步**：新增 `find_package(CURL)` 时同步更新 CI 依赖清单（否则 CI 挂）——依赖契约管理

## 下一步

1. 网络恢复后：下载 Ollama → 实现 `OllamaProvider`（libcurl 调 `/api/chat`）→ 一行替换注册 + 集成测试
2. POST /gateway/chat 接入 JWT 鉴权（当前与 auth 一样未强制）
3. 超时/重试/密钥管理（V2 范围）