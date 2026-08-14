#include "auth/mysql_user_repository.h"

#include <stdexcept>

namespace devpilot::auth {

drogon::Task<std::optional<uint64_t>> MysqlUserRepository::create_user(
    const std::string& username, const std::string& password_hash)
{
    try
    {
        const auto result = co_await db_->execSqlCoro(
            "INSERT INTO `user` (username, password_hash) VALUES (?, ?)",
            username, password_hash);
        co_return result.affectedRows() > 0
                    ? std::optional<uint64_t>{result.insertId()}
                    : std::nullopt;
    }
    catch (const std::exception& e)
    {
        // 唯一索引冲突（用户名重复）表现为数据库异常
        LOG_WARN << "create_user failed: " << e.what();
        co_return std::nullopt;
    }
}

drogon::Task<std::optional<UserRecord>> MysqlUserRepository::find_by_username(
    const std::string& username)
{
    const auto result = co_await db_->execSqlCoro(
        "SELECT id, username, password_hash, create_time FROM `user` WHERE username = ?",
        username);

    if (result.empty())
    {
        co_return std::nullopt;
    }
    const auto& row = result[0];
    UserRecord record;
    record.id = row["id"].as<uint64_t>();
    record.username = row["username"].as<std::string>();
    record.password_hash = row["password_hash"].as<std::string>();
    co_return record;
}

drogon::Task<std::optional<UserRecord>> MysqlUserRepository::find_by_id(uint64_t id)
{
    const auto result = co_await db_->execSqlCoro(
        "SELECT id, username, password_hash, create_time FROM `user` WHERE id = ?", id);

    if (result.empty())
    {
        co_return std::nullopt;
    }
    const auto& row = result[0];
    UserRecord record;
    record.id = row["id"].as<uint64_t>();
    record.username = row["username"].as<std::string>();
    record.password_hash = row["password_hash"].as<std::string>();
    co_return record;
}

} // namespace devpilot::auth