#include <gtest/gtest.h>
#include <drogon/utils/coroutine.h>

#include "auth/user_repository.h"

using devpilot::auth::InMemoryUserRepository;

TEST(UserRepositoryTest, CreateAndFindByUsername)
{
    InMemoryUserRepository repo;
    const auto id = drogon::sync_wait(repo.create_user("alice", "hash-alice"));
    ASSERT_TRUE(id.has_value());

    const auto found = drogon::sync_wait(repo.find_by_username("alice"));
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->id, *id);
    EXPECT_EQ(found->username, "alice");
    EXPECT_EQ(found->password_hash, "hash-alice");
}

TEST(UserRepositoryTest, CreateAndFindById)
{
    InMemoryUserRepository repo;
    const auto id = drogon::sync_wait(repo.create_user("bob", "hash-bob"));
    ASSERT_TRUE(id.has_value());

    const auto found = drogon::sync_wait(repo.find_by_id(*id));
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->username, "bob");
}

TEST(UserRepositoryTest, DuplicateUsernameRejected)
{
    InMemoryUserRepository repo;
    ASSERT_TRUE(drogon::sync_wait(repo.create_user("carol", "hash-1")).has_value());
    EXPECT_FALSE(drogon::sync_wait(repo.create_user("carol", "hash-2")).has_value());
}

TEST(UserRepositoryTest, MissingUserReturnsNullopt)
{
    InMemoryUserRepository repo;
    EXPECT_FALSE(drogon::sync_wait(repo.find_by_username("nobody")).has_value());
    EXPECT_FALSE(drogon::sync_wait(repo.find_by_id(999)).has_value());
}

TEST(UserRepositoryTest, IdsAreMonotonic)
{
    InMemoryUserRepository repo;
    const auto id1 = drogon::sync_wait(repo.create_user("dave", "h"));
    const auto id2 = drogon::sync_wait(repo.create_user("erin", "h"));
    ASSERT_TRUE(id1.has_value());
    ASSERT_TRUE(id2.has_value());
    EXPECT_LT(*id1, *id2);
}