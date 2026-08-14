#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "gateway/provider.h"

namespace devpilot::gateway {

// AI Gateway（SDD 第 4.3 节）：
//  - 模型选择与路由（请求指定 provider，Gateway 查表转发）
//  - 不负责 Agent 调度（那是 Agent Runtime 的职责）
//  - 超时/重试/密钥管理为 V2 范围（V1 最简化）
class Gateway
{
public:
    void register_provider(std::shared_ptr<IProvider> provider);

    // 按 provider 名转发请求；未注册的 provider 抛 std::out_of_range。
    ChatResult complete(const std::string& provider, const ChatRequest& req);

    // 已注册的 provider 名列表（供 API 查询能力）
    std::vector<std::string> providers() const;

private:
    std::unordered_map<std::string, std::shared_ptr<IProvider>> providers_;
};

} // namespace devpilot::gateway