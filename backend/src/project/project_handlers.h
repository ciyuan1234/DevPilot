#pragma once

#include <memory>
#include <string>

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/utils/coroutine.h>

namespace devpilot::project {

class IProjectRepository;

// POST /api/project/create {name, storage_type?, storage_reference?, language?}
// 需 Bearer token；返回 201 {project_id, name}
drogon::Task<drogon::HttpResponsePtr> handle_create_project(
    drogon::HttpRequestPtr req, std::shared_ptr<IProjectRepository> repo,
    const std::string& jwt_secret);

// GET /api/project/list —— 当前用户的项目列表（归属隔离由 user_id 保证）
drogon::Task<drogon::HttpResponsePtr> handle_list_projects(
    drogon::HttpRequestPtr req, std::shared_ptr<IProjectRepository> repo,
    const std::string& jwt_secret);

} // namespace devpilot::project