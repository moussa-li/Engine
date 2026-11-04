#include <gtest/gtest.h>

#include "DataBase/DataNodeFactory.hpp"
#include "DataBase/DataTree.hpp"

namespace EgLab::Platform
{
    namespace
    {
        class TestNode : public DataNode
        {
        public:
            TestNode(DataNodeId id, const Common::String& name) : DataNode(id, name)
            {
            }
        };
    } // namespace
} // namespace EgLab::Platform

TEST(DataNode, FactoryAssignsIdsAndTreeFindsAndRemovesNodes)
{
    using namespace EgLab;

    auto& factory = Platform::DataNodeFactory::instance();
    auto first = factory.create<Platform::TestNode>("first");
    auto second = factory.create<Platform::TestNode>("second");
    EXPECT_NE(first->getId(), 0);
    EXPECT_GT(second->getId(), first->getId());
    EXPECT_EQ(first->getName(), "first");

    Platform::DataTree tree("root");
    Common::SharedPtr<Platform::DataNode> firstNode = first;
    Common::SharedPtr<Platform::DataNode> secondNode = second;
    ASSERT_EQ(tree.addNode(firstNode), Common::Return::Succeed);
    ASSERT_EQ(tree.addNode(secondNode), Common::Return::Succeed);
    EXPECT_EQ(tree.findNode(first->getId()), firstNode);

    auto removed = tree.removeNode(first->getId());
    EXPECT_EQ(removed, firstNode);
    EXPECT_FALSE(tree.findNode(first->getId()));
    EXPECT_EQ(tree.getDataNodes().size(), 1);
}

TEST(DataNode, TreeRejectsDuplicateIds)
{
    using namespace EgLab;

    Platform::DataTree tree("root");
    Common::SharedPtr<Platform::DataNode> first =
        Common::makeShared<Platform::TestNode>(42, "first");
    Common::SharedPtr<Platform::DataNode> duplicate =
        Common::makeShared<Platform::TestNode>(42, "duplicate");

    EXPECT_EQ(tree.addNode(first), EgLab::Common::Return::Succeed);
    EXPECT_EQ(tree.addNode(duplicate), EgLab::Common::Return::Failed);
    EXPECT_EQ(tree.getDataNodes().size(), 1);
}

TEST(DataNode, TreeRejectsDuplicateNamesAcrossBranches)
{
    using namespace EgLab;

    Platform::DataTree tree("root");
    auto parent = Common::makeShared<Platform::TestNode>(1, "parent");
    auto first = Common::makeShared<Platform::TestNode>(2, "same-name");
    auto duplicate = Common::makeShared<Platform::TestNode>(3, "same-name");
    auto duplicateId = Common::makeShared<Platform::TestNode>(2, "different-name");

    ASSERT_EQ(tree.addNode(parent), Common::Return::Succeed);
    ASSERT_EQ(tree.addNode(first, parent->getId()), Common::Return::Succeed);
    EXPECT_EQ(tree.addNode(duplicate), Common::Return::Failed);
    EXPECT_EQ(tree.addNode(duplicateId), Common::Return::Failed);
    EXPECT_EQ(tree.findNode("same-name"), first);
    ASSERT_EQ(first->getParent(), parent.get());
    ASSERT_EQ(parent->getChildren().size(), 1);
}

TEST(DataNode, TreeUpdatePreservesParentAndChildren)
{
    using namespace EgLab;

    Platform::DataTree tree("root");
    auto parent = Common::makeShared<Platform::TestNode>(10, "parent");
    auto child = Common::makeShared<Platform::TestNode>(11, "child");
    ASSERT_EQ(tree.addNode(parent), Common::Return::Succeed);
    ASSERT_EQ(tree.addNode(child, parent->getId()), Common::Return::Succeed);

    auto replacement = Common::makeShared<Platform::TestNode>(10, "renamed-parent");
    ASSERT_EQ(tree.updateNode(parent->getId(), replacement), Common::Return::Succeed);
    EXPECT_FALSE(tree.findNode("parent"));
    EXPECT_EQ(tree.findNode("renamed-parent"), replacement);
    EXPECT_EQ(tree.findNode(child->getId()), child);
    EXPECT_EQ(replacement->getParent(), &tree);
    ASSERT_EQ(replacement->getChildren().size(), 1);
    EXPECT_EQ(child->getParent(), replacement.get());
    EXPECT_EQ(parent->getParent(), nullptr);
}