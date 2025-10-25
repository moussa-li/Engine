#include <gtest/gtest.h>

class TestCommon : public testing::Test {
protected:
    // nothing to setup
    virtual void SetUp() override {
    }
    virtual void TearDown() override {
    }
};