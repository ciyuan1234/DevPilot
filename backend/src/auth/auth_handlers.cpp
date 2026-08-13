#include "auth/auth_handlers.h"

#include <chrono>
#include <jsoncpp/json/json.h>

#include "auth/jwt.h"
#include "auth/user_repository.h"
#include "util/password_hash.h"

namespace devpilot::auth {
namespace {

constexpr auto kTokenTtl = std::chrono::hours{24};

drogon::HttpResponsePtr make_json_error(drogon::HttpStatusCode code, const std::string& message)
{
    Json::Value body;
    body["error"] = message;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    resp->setStatusCode(code);
    return resp;
}

// 密码强度校验：至少 8 位，含字母和数字（V1 基础策略，企业可加正则）
bool is_valid_password(const std::string& password)
{
    if (password.size() < 8)
    {
        return false;
    }
    bool has_alpha = false;
    bool has_digit = false;
    for (const char c : password)
    {
        if (std::isalpha(static_cast<unsigned char>(c)))
        {
            has_alpha = true;
        }
        if (std::isdigit(static_cast<unsigned char>(c)))
        {
            has_digit = true;
        }
    }
    return has_alpha && has_digit;
}

bool is_valid_username(const std::string& username)
{
    return !username.empty() && username.size() <= 64;
}

} // namespace

drogon::Task<drogon::HttpResponsePtr> handle_register(drogon::HttpRequestPtr req,
                                                      std::shared_ptr<IUserRepository> repo)
{
    auto json = req->getJsonObject();
    if (json == nullptr || !json->isMember("username") || !json->isMember("password"))
    {
        co_return make_json_error(drogon::k400BadRequest, "username and password are required");
    }

    const auto username = (*json)["username"].asString();
    const auto password = (*json)["password"].asString();
    if (!is_valid_username(username))
    {
        co_return make_json_error(drogon::k400BadRequest, "invalid username");
    }
    if (!is_valid_password(password))
    {
        co_return make_json_error(drogon::k400BadRequest,
                                  "password must be at least 8 chars with letters and digits");
    }

    const auto hash = util::hash_password(password);
    const auto id = repo->create_user(username, hash);
    if (!id.has_value())
    {
        co_return make_json_error(drogon::k409Conflict, "username already exists");
    }

    Json::Value body;
    body["user_id"] = *id;
    body["username"] = username;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    resp->setStatusCode(drogon::k201Created);
    co_return resp;
}

drogon::Task<drogon::HttpResponsePtr> handle_login(drogon::HttpRequestPtr req,
                                                   std::shared_ptr<IUserRepository> repo,
                                                   const std::string& jwt_secret)
{
    auto json = req->getJsonObject();
    if (json == nullptr || !json->isMember("username") || !json->isMember("password"))
    {
        co_return make_json_error(drogon::k400BadRequest, "username and password are required");
    }

    const auto username = (*json)["username"].asString();
    const auto password = (*json)["password"].asString();

    const auto user = repo->find_by_username(username);
    // 统一返回 401：不区分"用户不存在"与"密码错误"，防止用户名枚举
    if (!user.has_value() || !util::verify_password(password, user->password_hash))
    {
        co_return make_json_error(drogon::k401Unauthorized, "invalid credentials");
    }

    const auto token = sign_jwt(user->id, user->username, jwt_secret, kTokenTtl);

    Json::Value body;
    body["token"] = token;
    body["user_id"] = user->id;
    co_return drogon::HttpResponse::newHttpJsonResponse(body);
}

} // namespace devpilot::auth