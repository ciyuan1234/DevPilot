#include <gtest/gtest.h>
#include <drogon/utils/coroutine.h>

#include "project/project_repository.h"

using devpilot::project::InMemoryProjectRepository;

TEST(ProjectRepositoryTest, CreateAssignsMonotonicIds)
{
    InMemoryProjectRepository repo;
    const auto id1 = drogon::sync_wait(repo.create_project(1, "alpha", "local", "/tmp/a", "cpp"));
    const auto id2 = drogon::sync_wait(repo.create_project(1, "beta", "local", "/tmp/b", "cpp"));
    ASSERT_TRUE(id1.has_value());
    ASSERT_TRUE(id2.has_value());
    EXPECT_LT(*id1, *id2);
}

TEST(ProjectRepositoryTest, ListReturnsOwnProjectsOnly)
{
    InMemoryProjectRepository repo;
    drogon::sync_wait(repo.create_project(1, "mine", "local", "", "cpp"));
    drogon::sync_wait(repo.create_project(2, "other", "local", "", "go"));

    const auto mine = drogon::sync_wait(repo.list_by_user(1));
    ASSERT_EQ(mine.size(), 1);
    EXPECT_EQ(mine[0].name, "mine");
    EXPECT_EQ(mine[0].user_id, 1);

    const auto none = drogon::sync_wait(repo.list_by_user(3));
    EXPECT_TRUE(none.empty());
}

TEST(ProjectRepositoryTest, ListPreservesFieldsAndNewestFirst)
{
    InMemoryProjectRepository repo;
    const auto id1 = drogon::sync_wait(
        repo.create_project(1, "old", "object_storage", "s3://bucket/key", "python"));
    const auto id2 = drogon::sync_wait(
        repo.create_project(1, "new", "remote", "git@host:repo.git", "javascript"));

    const auto list = drogon::sync_wait(repo.list_by_user(1));
    ASSERT_EQ(list.size(), 2);
    EXPECT_EQ(list[0].name, "new");
    EXPECT_EQ(list[0].storage_type, "remote");
    EXPECT_EQ(list[0].storage_reference, "git@host:repo.git");
    EXPECT_EQ(list[1].name, "old");
    EXPECT_EQ(list[1].storage_type, "object_storage");
    EXPECT_EQ(list[1].language, "python");
}