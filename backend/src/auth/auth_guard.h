#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <drogon/HttpRequest.h>

namespace devpilot::auth {

// 从 Authorization: Bearer <token> 提取并校验 JWT。
// 成功返回 user_id；缺头/格式错/签名非法/过期均返回 std::nullopt（由 handler 回 401）。
// V1 用显式守卫而非中间件：规模小、可单测、依赖显式（复杂度与规模匹配）。
std::optional<uint64_t> require_auth(const drogon::HttpRequestPtr& req,
                                     const std::string& jwt_secret);

} // namespace devpilot::auth