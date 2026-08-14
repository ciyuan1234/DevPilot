#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <drogon/utils/coroutine.h>

namespace devpilot::project {

struct ProjectRecord
{
    uint64_t id;
    uint64_t user_id;
    std::string name;
    std::string storage_type;      // local | object_storage | remote
    std::string storage_reference; // 存储路径或 object key
    std::string language;          // 主语言，如 cpp / python / javascript
    std::chrono::system_clock::time_point create_time;
};

// 项目存储层接口：业务只依赖此抽象（同 IUserRepository 模式）。
class IProjectRepository
{
public:
    virtual ~IProjectRepository() = default;

    // 创建项目；返回新项目 id。
    virtual drogon::Task<std::optional<uint64_t>> create_project(
        uint64_t user_id, const std::string& name, const std::string& storage_type,
        const std::string& storage_reference, const std::string& language) = 0;

    // 某用户的项目列表（多租户隔离：永远按 user_id 过滤，不信任客户端）。
    virtual drogon::Task<std::vector<ProjectRecord>> list_by_user(uint64_t user_id) = 0;
};

// 内存实现：单测与演示，不持久化。
class InMemoryProjectRepository final : public IProjectRepository
{
public:
    drogon::Task<std::optional<uint64_t>> create_project(
        uint64_t user_id, const std::string& name, const std::string& storage_type,
        const std::string& storage_reference, const std::string& language) override;
    drogon::Task<std::vector<ProjectRecord>> list_by_user(uint64_t user_id) override;

private:
    std::unordered_map<uint64_t, ProjectRecord> by_id_;
    uint64_t next_id_ = 1;
};

} // namespace devpilot::project