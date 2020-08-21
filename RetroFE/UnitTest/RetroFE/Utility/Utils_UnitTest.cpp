#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <Utility/Utils.h>

class UtilsTest : public ::testing::Test
{
};

TEST_F(UtilsTest, ConvertsStringToInt)
{
    ASSERT_EQ(5, Utils::ConvertInt("5"));
}
