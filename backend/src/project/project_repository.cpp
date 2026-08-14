#include "project/project_repository.h"

namespace devpilot::project {

drogon::Task<std::optional<uint64_t>> InMemoryProjectRepository::create_project(
    uint64_t user_id, const std::string& name, const std::string& storage_type,
    const std::string& storage_reference, const std::string& language)
{
    ProjectRecord record;
    record.id = next_id_++;
    record.user_id = user_id;
    record.name = name;
    record.storage_type = storage_type;
    record.storage_reference = storage_reference;
    record.language = language;
    record.create_time = std::chrono::system_clock::now();
    by_id_[record.id] = std::move(record);
    co_return record.id;
}

drogon::Task<std::vector<ProjectRecord>> InMemoryProjectRepository::list_by_user(
    uint64_t user_id)
{
    std::vector<ProjectRecord> out;
    for (const auto& [id, record] : by_id_)
    {
        if (record.user_id == user_id)
        {
            out.push_back(record);
        }
    }
    std::sort(out.begin(), out.end(),
              [](const auto& a, const auto& b) { return a.id > b.id; });
    co_return out;
}

} // namespace devpilot::project