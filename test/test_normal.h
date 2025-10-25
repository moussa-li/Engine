#include <gtest/gtest.h>

class TestNormal : public testing::Test {
protected:
    // nothing to setup
    virtual void SetUp() override {
    }
    virtual void TearDown() override {
    }
};