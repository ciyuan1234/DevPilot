#include <gtest/gtest.h>

#include "auth/jwt.h"

using devpilot::auth::sign_jwt;
using devpilot::auth::verify_jwt;

constexpr char kSecret[] = "unit-test-secret";

TEST(JwtTest, SignAndVerifyRoundTrip)
{
    const auto token = sign_jwt(42, "alice", kSecret, std::chrono::seconds{3600});
    const auto uid = verify_jwt(token, kSecret);
    ASSERT_TRUE(uid.has_value());
    EXPECT_EQ(*uid, 42);
}

TEST(JwtTest, WrongSecretRejected)
{
    const auto token = sign_jwt(1, "alice", kSecret, std::chrono::seconds{3600});
    EXPECT_FALSE(verify_jwt(token, "other-secret").has_value());
}

TEST(JwtTest, TamperedTokenRejected)
{
    const auto token = sign_jwt(1, "alice", kSecret, std::chrono::seconds{3600});
    // 篡改 payload 段（第二段），伪造一个不同的 subject：签名校验必然失败
    std::string tampered = token;
    const auto first_dot = tampered.find('.');
    const auto second_dot = tampered.find('.', first_dot + 1);
    ASSERT_NE(first_dot, std::string::npos);
    ASSERT_NE(second_dot, std::string::npos);
    tampered.replace(first_dot + 1, second_dot - first_dot - 1, "eyJzdWIiOiIyIn0");
    EXPECT_FALSE(verify_jwt(tampered, kSecret).has_value());
}

TEST(JwtTest, ExpiredTokenRejected)
{
    const auto token = sign_jwt(1, "alice", kSecret, std::chrono::seconds{-1});
    EXPECT_FALSE(verify_jwt(token, kSecret).has_value());
}

TEST(JwtTest, GarbageInputRejected)
{
    EXPECT_FALSE(verify_jwt("not-a-jwt", kSecret).has_value());
    EXPECT_FALSE(verify_jwt("", kSecret).has_value());
    EXPECT_FALSE(verify_jwt("a.b.c", kSecret).has_value());
}
