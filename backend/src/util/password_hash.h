#pragma once

#include <string>

namespace devpilot::util {

// 对密码做 argon2id 哈希，返回 PHC 格式字符串（含盐与参数），如：
// $argon2id$v=19$m=65536,t=3,p=4$<salt>$<hash>
// 每次调用结果不同（盐随机）。绝不存储明文密码。
std::string hash_password(const std::string& password);

// 校验密码与存储的哈希是否匹配（从 encoded 中解析盐与参数）。
bool verify_password(const std::string& password, const std::string& encoded);

}