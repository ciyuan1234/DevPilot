#pragma once

#include <memory>

#include <drogon/orm/DbClient.h>
#include <drogon/utils/coroutine.h>

#include "project/project_repository.h"

namespace devpilot::project {

// MySQL 实现：参数化查询防注入；list 按 user_id 走 idx_user_id 索引。
class MysqlProjectRepository final : public IProjectRepository
{
public:
    explicit MysqlProjectRepository(drogon::orm::DbClientPtr db) : db_(std::move(db)) {}

    drogon::Task<std::optional<uint64_t>> create_project(
        uint64_t user_id, const std::string& name, const std::string& storage_type,
        const std::string& storage_reference, const std::string& language) override;
    drogon::Task<std::vector<ProjectRecord>> list_by_user(uint64_t user_id) override;

private:
    drogon::orm::DbClientPtr db_;
};

} // namespace devpilot::project