#include "auth/user_repository.h"

namespace devpilot::auth {

drogon::Task<std::optional<uint64_t>> InMemoryUserRepository::create_user(
    const std::string& username, const std::string& password_hash)
{
    if (by_username_.contains(username))
    {
        co_return std::nullopt; // 用户名冲突
    }

    const uint64_t id = next_id_++;
    Entry entry;
    entry.record.id = id;
    entry.record.username = username;
    entry.record.password_hash = password_hash;
    entry.record.create_time = std::chrono::system_clock::now();

    by_username_.emplace(username, entry);
    by_id_.emplace(id, std::move(entry));
    co_return id;
}

drogon::Task<std::optional<UserRecord>> InMemoryUserRepository::find_by_username(
    const std::string& username)
{
    const auto it = by_username_.find(username);
    if (it == by_username_.end())
    {
        co_return std::nullopt;
    }
    co_return it->second.record;
}

drogon::Task<std::optional<UserRecord>> InMemoryUserRepository::find_by_id(uint64_t id)
{
    const auto it = by_id_.find(id);
    if (it == by_id_.end())
    {
        co_return std::nullopt;
    }
    co_return it->second.record;
}

} // namespace devpilot::auth