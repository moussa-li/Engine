#pragma once

#include <gtest/gtest.h>

class TestMesh : public testing::Test
{
protected:
    // nothing to setup
    virtual void SetUp() override
    {
    }
    virtual void TearDown() override
    {
    }
};