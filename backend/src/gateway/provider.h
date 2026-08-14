#pragma once

#include <string>
#include <vector>

namespace devpilot::gateway {

struct ChatMessage
{
    std::string role;    // "user" | "system" | "assistant"
    std::string content;
};

struct ChatRequest
{
    std::string model;
    std::vector<ChatMessage> messages;
    double temperature = 0.7;
};

struct ChatResult
{
    std::string content;
    int prompt_tokens = 0;
    int completion_tokens = 0;
};

// 模型 Provider 统一接口（SDD 第 4.5 节）。
// 业务代码只依赖此抽象；新增模型 = 新增实现，业务零改动。
class IProvider
{
public:
    virtual ~IProvider() = default;

    // Provider 标识，用于 Gateway 路由（如 "ollama" / "openai"）
    virtual std::string name() const = 0;

    // 调用模型完成一次对话。实现失败时抛 std::runtime_error。
    virtual ChatResult complete(const ChatRequest& req) = 0;
};

} // namespace devpilot::gateway