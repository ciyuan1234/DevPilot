#pragma once

#include <string>

#include "gateway/provider.h"

namespace devpilot::gateway {

// 内存模拟 Provider：不调用真实模型，返回固定回复。
// 用途：单测 + 开发调试（不依赖网络），接口与真实 Provider 完全一致。
class FakeProvider final : public IProvider
{
public:
    explicit FakeProvider(std::string name = "fake") : name_(std::move(name)) {}

    std::string name() const override { return name_; }

    // 模拟回复：统计请求 token 数 + 固定内容
    ChatResult complete(const ChatRequest& req) override
    {
        int prompt_tokens = 0;
        for (const auto& m : req.messages)
        {
            prompt_tokens += static_cast<int>(m.content.size() / 4) + 1;
        }
        return ChatResult{
            .content = "fake reply to: " + (req.messages.empty() ? "" : req.messages.back().content),
            .prompt_tokens = prompt_tokens,
            .completion_tokens = 8,
        };
    }

private:
    std::string name_;
};

} // namespace devpilot::gateway