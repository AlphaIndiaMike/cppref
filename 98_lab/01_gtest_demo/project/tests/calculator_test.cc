#include "calculator.h"

#include <gtest/gtest.h>

namespace my_project {
namespace {

class CalculatorTest : public ::testing::Test {
 protected:
    Calculator calc_;
};

// =============================================================================
// Add Tests
// =============================================================================

TEST_F(CalculatorTest, AddPositiveNumbers) {
    EXPECT_EQ(calc_.Add(2, 3), 5);
}

TEST_F(CalculatorTest, AddNegativeNumbers) {
    EXPECT_EQ(calc_.Add(-2, -3), -5);
}

TEST_F(CalculatorTest, AddMixedNumbers) {
    EXPECT_EQ(calc_.Add(-2, 5), 3);
}

TEST_F(CalculatorTest, AddZero) {
    EXPECT_EQ(calc_.Add(5, 0), 5);
    EXPECT_EQ(calc_.Add(0, 5), 5);
}

// =============================================================================
// Subtract Tests
// =============================================================================

TEST_F(CalculatorTest, SubtractPositiveNumbers) {
    EXPECT_EQ(calc_.Subtract(5, 3), 2);
}

TEST_F(CalculatorTest, SubtractResultNegative) {
    EXPECT_EQ(calc_.Subtract(3, 5), -2);
}

// =============================================================================
// Multiply Tests
// =============================================================================

TEST_F(CalculatorTest, MultiplyPositiveNumbers) {
    EXPECT_EQ(calc_.Multiply(3, 4), 12);
}

TEST_F(CalculatorTest, MultiplyByZero) {
    EXPECT_EQ(calc_.Multiply(5, 0), 0);
}

TEST_F(CalculatorTest, MultiplyNegativeNumbers) {
    EXPECT_EQ(calc_.Multiply(-3, -4), 12);
    EXPECT_EQ(calc_.Multiply(-3, 4), -12);
}

// =============================================================================
// Divide Tests
// =============================================================================

TEST_F(CalculatorTest, DivideEvenNumbers) {
    EXPECT_DOUBLE_EQ(calc_.Divide(10, 2), 5.0);
}

TEST_F(CalculatorTest, DivideWithRemainder) {
    EXPECT_DOUBLE_EQ(calc_.Divide(7, 2), 3.5);
}

TEST_F(CalculatorTest, DivideByZeroThrows) {
    EXPECT_THROW(calc_.Divide(5, 0), std::invalid_argument);
}

}  // namespace
}  // namespace my_project
