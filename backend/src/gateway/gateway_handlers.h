#pragma once

#include <memory>
#include <string>

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <drogon/utils/coroutine.h>

namespace devpilot::gateway {

class Gateway;

// POST /api/gateway/chat {provider, model, messages:[{role,content}]}
drogon::Task<drogon::HttpResponsePtr> handle_chat(drogon::HttpRequestPtr req,
                                                  std::shared_ptr<Gateway> gw);

// GET /api/gateway/providers —— 查询可用模型能力（本版 Drogon 不支持同步返回式
// handler，必须用"回调式"签名：void(const HttpRequestPtr&, std::function<void(const HttpResponsePtr&)>&&)）
void handle_providers(const drogon::HttpRequestPtr&,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                      std::shared_ptr<Gateway> gw);

} // namespace devpilot::gateway