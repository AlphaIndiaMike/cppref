#include "string_utils.h"

#include <gtest/gtest.h>

namespace my_project {
namespace {

// =============================================================================
// ToUpper Tests
// =============================================================================

TEST(StringUtilsTest, ToUpperConvertsLowercase) {
    EXPECT_EQ(ToUpper("hello"), "HELLO");
}

TEST(StringUtilsTest, ToUpperPreservesUppercase) {
    EXPECT_EQ(ToUpper("HELLO"), "HELLO");
}

TEST(StringUtilsTest, ToUpperHandlesMixedCase) {
    EXPECT_EQ(ToUpper("HeLLo WoRLd"), "HELLO WORLD");
}

TEST(StringUtilsTest, ToUpperHandlesEmptyString) {
    EXPECT_EQ(ToUpper(""), "");
}

// =============================================================================
// ToLower Tests
// =============================================================================

TEST(StringUtilsTest, ToLowerConvertsUppercase) {
    EXPECT_EQ(ToLower("HELLO"), "hello");
}

TEST(StringUtilsTest, ToLowerPreservesLowercase) {
    EXPECT_EQ(ToLower("hello"), "hello");
}

TEST(StringUtilsTest, ToLowerHandlesMixedCase) {
    EXPECT_EQ(ToLower("HeLLo WoRLd"), "hello world");
}

// =============================================================================
// Trim Tests
// =============================================================================

TEST(StringUtilsTest, TrimRemovesLeadingSpaces) {
    EXPECT_EQ(Trim("   hello"), "hello");
}

TEST(StringUtilsTest, TrimRemovesTrailingSpaces) {
    EXPECT_EQ(Trim("hello   "), "hello");
}

TEST(StringUtilsTest, TrimRemovesBothEnds) {
    EXPECT_EQ(Trim("   hello   "), "hello");
}

TEST(StringUtilsTest, TrimPreservesInternalSpaces) {
    EXPECT_EQ(Trim("  hello world  "), "hello world");
}

TEST(StringUtilsTest, TrimHandlesEmptyString) {
    EXPECT_EQ(Trim(""), "");
}

TEST(StringUtilsTest, TrimHandlesWhitespaceOnly) {
    EXPECT_EQ(Trim("    "), "");
}

TEST(StringUtilsTest, TrimHandlesTabs) {
    EXPECT_EQ(Trim("\t\thello\t\t"), "hello");
}

// =============================================================================
// IsBlank Tests
// =============================================================================

TEST(StringUtilsTest, IsBlankReturnsTrueForEmpty) {
    EXPECT_TRUE(IsBlank(""));
}

TEST(StringUtilsTest, IsBlankReturnsTrueForSpaces) {
    EXPECT_TRUE(IsBlank("    "));
}

TEST(StringUtilsTest, IsBlankReturnsTrueForTabs) {
    EXPECT_TRUE(IsBlank("\t\t"));
}

TEST(StringUtilsTest, IsBlankReturnsFalseForText) {
    EXPECT_FALSE(IsBlank("hello"));
}

TEST(StringUtilsTest, IsBlankReturnsFalseForTextWithSpaces) {
    EXPECT_FALSE(IsBlank("  hello  "));
}

}  // namespace
}  // namespace my_project
