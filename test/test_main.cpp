#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <gtest/gtest.h>
#include <stdlib.h>

#include "test_normal.h"

int main(int argc, char **argv)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}