#include "project/mysql_project_repository.h"

#include <stdexcept>

namespace devpilot::project {

drogon::Task<std::optional<uint64_t>> MysqlProjectRepository::create_project(
    uint64_t user_id, const std::string& name, const std::string& storage_type,
    const std::string& storage_reference, const std::string& language)
{
    try
    {
        const auto result = co_await db_->execSqlCoro(
            "INSERT INTO `project` (user_id, name, storage_type, storage_reference, language) "
            "VALUES (?, ?, ?, ?, ?)",
            user_id, name, storage_type, storage_reference, language);
        co_return result.affectedRows() > 0
                    ? std::optional<uint64_t>{result.insertId()}
                    : std::nullopt;
    }
    catch (const std::exception& e)
    {
        LOG_ERROR << "create_project failed: " << e.what();
        co_return std::nullopt;
    }
}

drogon::Task<std::vector<ProjectRecord>> MysqlProjectRepository::list_by_user(
    uint64_t user_id)
{
    const auto result = co_await db_->execSqlCoro(
        "SELECT id, user_id, name, storage_type, storage_reference, language, create_time "
        "FROM `project` WHERE user_id = ? ORDER BY id DESC",
        user_id);

    std::vector<ProjectRecord> out;
    out.reserve(result.size());
    for (const auto& row : result)
    {
        ProjectRecord record;
        record.id = row["id"].as<uint64_t>();
        record.user_id = row["user_id"].as<uint64_t>();
        record.name = row["name"].as<std::string>();
        record.storage_type = row["storage_type"].as<std::string>();
        record.storage_reference = row["storage_reference"].as<std::string>();
        record.language = row["language"].as<std::string>();
        out.push_back(std::move(record));
    }
    co_return out;
}

} // namespace devpilot::project