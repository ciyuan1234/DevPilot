#include <gtest/gtest.h>

#include "util/file_validate.h"

using devpilot::util::validate_source_file_name;

TEST(FileValidateTest, AcceptsCommonSourceExtensions)
{
    EXPECT_EQ(validate_source_file_name("main.cpp"), "");
    EXPECT_EQ(validate_source_file_name("foo.h"), "");
    EXPECT_EQ(validate_source_file_name("bar.hpp"), "");
    EXPECT_EQ(validate_source_file_name("baz.cxx"), "");
    EXPECT_EQ(validate_source_file_name("qux.c"), "");
}

TEST(FileValidateTest, AcceptsExtensionCaseInsensitively)
{
    EXPECT_EQ(validate_source_file_name("Main.CPP"), "");
    EXPECT_EQ(validate_source_file_name("Foo.H"), "");
}

TEST(FileValidateTest, RejectsEmptyAndWhitespaceOnly)
{
    EXPECT_FALSE(validate_source_file_name("").empty());
    EXPECT_FALSE(validate_source_file_name("  ").empty());
}

TEST(FileValidateTest, RejectsPathSeparators)
{
    EXPECT_FALSE(validate_source_file_name("a/b.cpp").empty());
    EXPECT_FALSE(validate_source_file_name("..\\evil.cpp").empty());
}

TEST(FileValidateTest, RejectsDotAndDotDot)
{
    EXPECT_FALSE(validate_source_file_name(".").empty());
    EXPECT_FALSE(validate_source_file_name("..").empty());
}

TEST(FileValidateTest, RejectsOversizedName)
{
    const std::string long_name(256, 'a');
    EXPECT_FALSE(validate_source_file_name(long_name + ".cpp").empty());
}

TEST(FileValidateTest, RejectsDisallowedExtensions)
{
    EXPECT_FALSE(validate_source_file_name("evil.txt").empty());
    EXPECT_FALSE(validate_source_file_name("run.sh").empty());
    EXPECT_FALSE(validate_source_file_name("script.exe").empty());
    EXPECT_FALSE(validate_source_file_name("README").empty());
}
