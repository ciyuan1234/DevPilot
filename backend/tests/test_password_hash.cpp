#include <gtest/gtest.h>

#include "util/password_hash.h"

using devpilot::util::hash_password;
using devpilot::util::verify_password;

TEST(PasswordHashTest, CorrectPasswordVerifies)
{
    const std::string encoded = hash_password("s3cret-p@ss");
    EXPECT_FALSE(encoded.empty());
    EXPECT_TRUE(verify_password("s3cret-p@ss", encoded));
}

TEST(PasswordHashTest, WrongPasswordRejected)
{
    const std::string encoded = hash_password("s3cret-p@ss");
    EXPECT_FALSE(verify_password("wrong-password", encoded));
}

TEST(PasswordHashTest, SamePasswordProducesDifferentSalt)
{
    const std::string a = hash_password("same-password");
    const std::string b = hash_password("same-password");
    EXPECT_NE(a, b);
    EXPECT_TRUE(verify_password("same-password", a));
    EXPECT_TRUE(verify_password("same-password", b));
}

TEST(PasswordHashTest, PhcFormatHasArgon2idPrefix)
{
    const std::string encoded = hash_password("x");
    EXPECT_EQ(encoded.rfind("$argon2id$", 0), 0);
}

TEST(PasswordHashTest, EmptyPasswordIsValidInputButVerifyFails)
{
    const std::string encoded = hash_password("");
    EXPECT_TRUE(verify_password("", encoded));
    EXPECT_FALSE(verify_password(" ", encoded));
}

TEST(PasswordHashTest, RejectsMalformedEncoded)
{
    EXPECT_FALSE(verify_password("anything", "not-a-valid-hash"));
    EXPECT_FALSE(verify_password("anything", ""));
}
