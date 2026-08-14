#include "gateway/gateway_handlers.h"

#include <jsoncpp/json/json.h>
#include <stdexcept>

#include "auth/auth_guard.h"
#include "gateway/gateway.h"

namespace devpilot::gateway {
namespace {

drogon::HttpResponsePtr make_json_error(drogon::HttpStatusCode code, const std::string& message)
{
    Json::Value body;
    body["error"] = message;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
    resp->setStatusCode(code);
    return resp;
}

} // namespace

drogon::Task<drogon::HttpResponsePtr> handle_chat(drogon::HttpRequestPtr req,
                                                  std::shared_ptr<Gateway> gw,
                                                  const std::string& jwt_secret)
{
    if (!devpilot::auth::require_auth(req, jwt_secret).has_value())
    {
        co_return make_json_error(drogon::k401Unauthorized, "unauthorized");
    }

    auto json = req->getJsonObject();
    if (json == nullptr || !json->isMember("provider"))
    {
        co_return make_json_error(drogon::k400BadRequest, "provider is required");
    }

    const auto provider = (*json)["provider"].asString();
    ChatRequest chat_req;
    chat_req.model = json->isMember("model") ? (*json)["model"].asString() : "";

    if (json->isMember("messages") && (*json)["messages"].isArray())
    {
        for (const auto& m : (*json)["messages"])
        {
            ChatMessage msg;
            msg.role = m.isMember("role") ? m["role"].asString() : "user";
            msg.content = m.isMember("content") ? m["content"].asString() : "";
            chat_req.messages.push_back(std::move(msg));
        }
    }
    if (chat_req.messages.empty())
    {
        co_return make_json_error(drogon::k400BadRequest, "messages must not be empty");
    }

    try
    {
        const auto result = gw->complete(provider, chat_req);
        Json::Value body;
        body["provider"] = provider;
        body["model"] = chat_req.model;
        body["content"] = result.content;
        body["prompt_tokens"] = result.prompt_tokens;
        body["completion_tokens"] = result.completion_tokens;
        co_return drogon::HttpResponse::newHttpJsonResponse(body);
    }
    catch (const std::out_of_range&)
    {
        co_return make_json_error(drogon::k404NotFound, "unknown provider: " + provider);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR << "gateway chat failed: " << e.what();
        co_return make_json_error(drogon::k502BadGateway, "provider call failed");
    }
}

void handle_providers(const drogon::HttpRequestPtr& req,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                      std::shared_ptr<Gateway> gw,
                      const std::string& jwt_secret)
{
    if (!devpilot::auth::require_auth(req, jwt_secret).has_value())
    {
        callback(make_json_error(drogon::k401Unauthorized, "unauthorized"));
        return;
    }

    Json::Value body;
    for (const auto& name : gw->providers())
    {
        body.append(name);
    }
    callback(drogon::HttpResponse::newHttpJsonResponse(body));
}

} // namespace devpilot::gateway