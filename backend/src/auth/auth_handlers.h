#pragma once

#include <memory>
#include <string>

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <drogon/utils/coroutine.h>

namespace devpilot::auth {

class IUserRepository;

// 注册：POST /api/auth/register {username, password}
drogon::Task<drogon::HttpResponsePtr> handle_register(drogon::HttpRequestPtr req,
                                                      std::shared_ptr<IUserRepository> repo);

// 登录：POST /api/auth/login {username, password}
drogon::Task<drogon::HttpResponsePtr> handle_login(drogon::HttpRequestPtr req,
                                                   std::shared_ptr<IUserRepository> repo,
                                                   const std::string& jwt_secret);

} // namespace devpilot::auth
