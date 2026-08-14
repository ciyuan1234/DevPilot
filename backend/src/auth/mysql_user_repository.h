#pragma once

#include <drogon/orm/DbClient.h>
#include <drogon/utils/coroutine.h>

#include "auth/user_repository.h"

namespace devpilot::auth {

// MySQL 持久化实现：通过 Drogon DbClient 访问数据库。
// 全部使用参数化查询（? 占位符）防止 SQL 注入。
// 注意：execSqlCoro 返回 SqlAwaiter（可等待对象），必须 co_await 取 Result，
//       因此这些方法本身是协程（Task 返回）。
class MysqlUserRepository final : public IUserRepository
{
public:
    explicit MysqlUserRepository(std::shared_ptr<drogon::orm::DbClient> db) : db_(std::move(db)) {}

    drogon::Task<std::optional<uint64_t>> create_user(const std::string& username,
                                                      const std::string& password_hash) override;
    drogon::Task<std::optional<UserRecord>> find_by_username(const std::string& username) override;
    drogon::Task<std::optional<UserRecord>> find_by_id(uint64_t id) override;

private:
    std::shared_ptr<drogon::orm::DbClient> db_;
};

} // namespace devpilot::auth
