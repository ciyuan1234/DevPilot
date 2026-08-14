#include "auth/auth_guard.h"

#include <string_view>

#include "auth/jwt.h"

namespace devpilot::auth {

std::optional<uint64_t> require_auth(const drogon::HttpRequestPtr& req,
                                     const std::string& jwt_secret)
{
    constexpr std::string_view kBearer = "Bearer ";
    const auto& header = req->getHeader("Authorization");

    if (header.size() <= kBearer.size() ||
        header.compare(0, kBearer.size(), kBearer) != 0)
    {
        return std::nullopt;
    }
    return verify_jwt(header.substr(kBearer.size()), jwt_secret);
}

} // namespace devpilot::auth