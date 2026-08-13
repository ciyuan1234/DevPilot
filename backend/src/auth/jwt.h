#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

namespace devpilot::auth {

// 使用 HMAC-SHA256 签发/校验 JWT（无状态认证令牌）。
// 密钥为服务器机密：泄露即等于能伪造任意用户的 token。
std::string sign_jwt(uint64_t user_id, const std::string& username,
                     const std::string& secret, std::chrono::seconds ttl);

// 校验 token：返回其中的 user_id；签名非法或过期返回 std::nullopt。
std::optional<uint64_t> verify_jwt(const std::string& token, const std::string& secret);

}
