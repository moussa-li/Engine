#include "test_normal.h"
#include <stdio.h>
#include "Basic.shader.h"

long a = BASIC_SHADER__SIZE;
TEST_F(TestNormal, easyTest)
{
    EXPECT_EQ(1, 1);
}

TEST_F(TestNormal, easyTest2)
{
    std::cout << a << std::endl;
}