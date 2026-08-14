#include "project/project_handlers.h"

#include <jsoncpp/json/json.h>
#include <stdexcept>
#include <string>

#include "auth/auth_guard.h"
#include "project/project_repository.h"

namespace devpilot::project {
namespace {

constexpr std::string_view kStorageTypes[] = {"local", "object_storage", "remote"};
constexpr size_t kMaxNameLen = 128;

drogon::HttpResponsePtr make_json_error(drogon::HttpStatusCode code, const std::string& message)
{
    Json::Value body;
    body["error"] = message;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    resp->setStatusCode(code);
    return resp;
}

} // namespace

drogon::Task<drogon::HttpResponsePtr> handle_create_project(
    drogon::HttpRequestPtr req, std::shared_ptr<IProjectRepository> repo,
    const std::string& jwt_secret)
{
    const auto user_id = devpilot::auth::require_auth(req, jwt_secret);
    if (!user_id.has_value())
    {
        co_return make_json_error(drogon::k401Unauthorized, "unauthorized");
    }

    auto json = req->getJsonObject();
    if (json == nullptr || !json->isMember("name"))
    {
        co_return make_json_error(drogon::k400BadRequest, "name is required");
    }

    const auto name = (*json)["name"].asString();
    if (name.empty() || name.size() > kMaxNameLen)
    {
        co_return make_json_error(drogon::k400BadRequest, "name must be 1-128 chars");
    }

    // 默认 local；显式传入时必须是枚举内的合法值
    std::string storage_type = "local";
    if (json->isMember("storage_type"))
    {
        storage_type = (*json)["storage_type"].asString();
        const bool valid =
            std::find(std::begin(kStorageTypes), std::end(kStorageTypes), storage_type) !=
            std::end(kStorageTypes);
        if (!valid)
        {
            co_return make_json_error(drogon::k400BadRequest,
                                      "storage_type must be local|object_storage|remote");
        }
    }

    const auto storage_reference =
        json->isMember("storage_reference") ? (*json)["storage_reference"].asString() : "";
    const auto language = json->isMember("language") ? (*json)["language"].asString() : "";

    const auto id = co_await repo->create_project(*user_id, name, storage_type,
                                                  storage_reference, language);
    if (!id.has_value())
    {
        co_return make_json_error(drogon::k500InternalServerError, "create project failed");
    }

    Json::Value body;
    body["project_id"] = *id;
    body["name"] = name;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    resp->setStatusCode(drogon::k201Created);
    co_return resp;
}

drogon::Task<drogon::HttpResponsePtr> handle_list_projects(
    drogon::HttpRequestPtr req, std::shared_ptr<IProjectRepository> repo,
    const std::string& jwt_secret)
{
    const auto user_id = devpilot::auth::require_auth(req, jwt_secret);
    if (!user_id.has_value())
    {
        co_return make_json_error(drogon::k401Unauthorized, "unauthorized");
    }

    const auto projects = co_await repo->list_by_user(*user_id);

    Json::Value body = Json::arrayValue;
    for (const auto& p : projects)
    {
        Json::Value item;
        item["project_id"] = p.id;
        item["name"] = p.name;
        item["storage_type"] = p.storage_type;
        item["language"] = p.language;
        body.append(std::move(item));
    }
    co_return drogon::HttpResponse::newHttpJsonResponse(body);
}

} // namespace devpilot::project