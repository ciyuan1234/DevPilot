#include <drogon/drogon.h>
#include <drogon/orm/DbClient.h>

#include <cstdlib>
#include <memory>

#include "auth/auth_handlers.h"
#include "auth/mysql_user_repository.h"
#include "gateway/fake_provider.h"
#include "gateway/gateway.h"
#include "gateway/gateway_handlers.h"

namespace {

std::shared_ptr<devpilot::auth::IUserRepository> g_user_repo;
std::shared_ptr<devpilot::gateway::Gateway> g_gateway;
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

drogon::Task<drogon::HttpResponsePtr> do_chat(drogon::HttpRequestPtr req)
{
    return devpilot::gateway::handle_chat(req, g_gateway, g_jwt_secret);
}

void do_providers(const drogon::HttpRequestPtr& req,
                  std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    devpilot::gateway::handle_providers(req, std::move(callback), g_gateway, g_jwt_secret);
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

    drogon::app().loadConfigFile("config.json");

    // 注意：getDbClient 必须在框架 run 之后才可用（Drogon 契约）。
    // 故在 registerBeginningAdvice 回调中初始化全局依赖。
    drogon::app().registerBeginningAdvice(
        []()
        {
            auto db = drogon::app().getDbClient("devpilot_db");
            g_user_repo = std::make_shared<devpilot::auth::MysqlUserRepository>(db);
            g_jwt_secret = jwt_secret();

            // AI Gateway：注册 Provider。当前用 Fake（Mock）；Ollama 就绪后换成 OllamaProvider。
            g_gateway = std::make_shared<devpilot::gateway::Gateway>();
            g_gateway->register_provider(std::make_shared<devpilot::gateway::FakeProvider>());
        });

    drogon::app().registerHandler("/api/auth/register", &do_register);
    drogon::app().registerHandler("/api/auth/login", &do_login);
    drogon::app().registerHandler("/api/gateway/chat", &do_chat);
    drogon::app().registerHandler("/api/gateway/providers", &do_providers);

    drogon::app()
        .addListener("0.0.0.0", 8080)
        .run();

    return 0;
}