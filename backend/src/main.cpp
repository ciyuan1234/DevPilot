#include <drogon/drogon.h>

#include <cstdlib>
#include <memory>

#include "auth/auth_handlers.h"
#include "auth/user_repository.h"

namespace {

std::shared_ptr<devpilot::auth::IUserRepository> g_user_repo;
std::string g_jwt_secret;

std::string jwt_secret()
{
    const char* env = std::getenv("DEVPILOT_JWT_SECRET");
    if (env != nullptr && *env != '\0')
    {
        return env;
    }
    // 仅本地开发默认值；生产必须配置环境变量
    LOG_WARN << "DEVPILOT_JWT_SECRET not set, using insecure default";
    return "dev-default-secret-do-not-use-in-prod";
}

// Drogon 协程 handler 要求精确签名 Task<HttpResponsePtr>(HttpRequestPtr)（按值），
// 不能是带捕获的 lambda 或 const& 参数（FunctionTraits 特化匹配），故用自由函数。
drogon::Task<drogon::HttpResponsePtr> do_register(drogon::HttpRequestPtr req)
{
    return devpilot::auth::handle_register(req, g_user_repo);
}
drogon::Task<drogon::HttpResponsePtr> do_login(drogon::HttpRequestPtr req)
{
    return devpilot::auth::handle_login(req, g_user_repo, g_jwt_secret);
}

} // namespace

int main()
{
    drogon::app().registerHandler(
        "/api/health",
        [](const drogon::HttpRequestPtr&,
           std::function<void(const drogon::HttpResponsePtr&)>&& callback)
        {
            Json::Value json;
            json["status"] = "ok";

            auto response = drogon::HttpResponse::newHttpJsonResponse(json);
            callback(response);
        }
    );

    g_user_repo = std::make_shared<devpilot::auth::InMemoryUserRepository>();
    g_jwt_secret = jwt_secret();

    drogon::app().registerHandler("/api/auth/register", &do_register);
    drogon::app().registerHandler("/api/auth/login", &do_login);

    drogon::app()
        .addListener("0.0.0.0", 8080)
        .run();

    return 0;
}