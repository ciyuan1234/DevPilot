#include "gateway/gateway.h"

#include <algorithm>
#include <stdexcept>

namespace devpilot::gateway {

void Gateway::register_provider(std::shared_ptr<IProvider> provider)
{
    providers_[provider->name()] = std::move(provider);
}

ChatResult Gateway::complete(const std::string& provider, const ChatRequest& req)
{
    const auto it = providers_.find(provider);
    if (it == providers_.end())
    {
        throw std::out_of_range("unknown provider: " + provider);
    }
    return it->second->complete(req);
}

std::vector<std::string> Gateway::providers() const
{
    std::vector<std::string> names;
    names.reserve(providers_.size());
    for (const auto& [name, _] : providers_)
    {
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());
    return names;
}

} // namespace devpilot::gateway