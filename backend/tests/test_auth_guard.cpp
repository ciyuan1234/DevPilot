#include <gtest/gtest.h>

#include <chrono>
#include <memory>

#include <drogon/HttpRequest.h>
#include <drogon/HttpTypes.h>

#include "auth/auth_guard.h"
#include "auth/jwt.h"

using devpilot::auth::require_auth;
using devpilot::auth::sign_jwt;

namespace {

constexpr char kSecret[] = "test-secret";

drogon::HttpRequestPtr make_request(const std::string& auth_header)
{
    auto req = drogon::HttpRequest::newHttpRequest();
    if (!auth_header.empty())
    {
        req->addHeader("Authorization", auth_header);
    }
    return req;
}

} // namespace

TEST(AuthGuardTest, MissingHeaderRejected)
{
    EXPECT_FALSE(require_auth(make_request(""), kSecret).has_value());
}

TEST(AuthGuardTest, WrongSchemeRejected)
{
    EXPECT_FALSE(require_auth(make_request("Basic dXNlcjpwYXNz"), kSecret).has_value());
    EXPECT_FALSE(require_auth(make_request("Bearer"), kSecret).has_value());
}

TEST(AuthGuardTest, GarbageTokenRejected)
{
    EXPECT_FALSE(require_auth(make_request("Bearer not.a.jwt"), kSecret).has_value());
}

TEST(AuthGuardTest, ValidTokenAccepted)
{
    const auto token = sign_jwt(42, "alice", kSecret, std::chrono::hours{1});
    const auto uid = require_auth(make_request("Bearer " + token), kSecret);
    ASSERT_TRUE(uid.has_value());
    EXPECT_EQ(*uid, 42);
}

TEST(AuthGuardTest, WrongSecretRejected)
{
    const auto token = sign_jwt(42, "alice", "other-secret", std::chrono::hours{1});
    EXPECT_FALSE(require_auth(make_request("Bearer " + token), kSecret).has_value());
}

TEST(AuthGuardTest, ExpiredTokenRejected)
{
    const auto token = sign_jwt(42, "alice", kSecret, std::chrono::seconds{-1});
    EXPECT_FALSE(require_auth(make_request("Bearer " + token), kSecret).has_value());
}