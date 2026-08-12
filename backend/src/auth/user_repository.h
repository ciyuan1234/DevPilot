#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

namespace devpilot::auth {

struct UserRecord
{
    uint64_t id;
    std::string username;
    std::string password_hash;
    std::chrono::system_clock::time_point create_time;
};

// 用户存储层接口：业务代码只依赖此抽象，不关心底层是 MySQL 还是内存。
class IUserRepository
{
public:
    virtual ~IUserRepository() = default;

    // 注册用户；用户名冲突时返回 std::nullopt。成功后返回新用户 id。
    virtual std::optional<uint64_t> create_user(const std::string& username,
                                                const std::string& password_hash) = 0;

    virtual std::optional<UserRecord> find_by_username(const std::string& username) = 0;

    virtual std::optional<UserRecord> find_by_id(uint64_t id) = 0;
};

// 内存实现：用于单测与演示分层，不持久化。
class InMemoryUserRepository final : public IUserRepository
{
public:
    std::optional<uint64_t> create_user(const std::string& username,
                                        const std::string& password_hash) override;
    std::optional<UserRecord> find_by_username(const std::string& username) override;
    std::optional<UserRecord> find_by_id(uint64_t id) override;

private:
    struct Entry
    {
        UserRecord record;
    };

    std::unordered_map<std::string, Entry> by_username_;
    std::unordered_map<uint64_t, Entry> by_id_;
    uint64_t next_id_ = 1;
};

} // namespace devpilot::auth