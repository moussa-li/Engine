#include <gtest/gtest.h>

class TestCommon : public testing::Test
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

class TestList : public testing::Test
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

class TestHashMap : public testing::Test
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