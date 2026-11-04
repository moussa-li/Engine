#include "DataBase/DataTree.hpp"

#include <mutex>

namespace EgLab::Platform
{
    namespace
    {
        Common::SharedPtr<DataNode> findNodeById(
            const Common::DynamicArray<Common::SharedPtr<TreeNode>>& nodes, DataNodeId id)
        {
            for (const auto& node : nodes)
            {
                if (node->getId() == id)
                {
                    return Common::dynamicSharedPtrCast<DataNode>(node);
                }
                if (auto descendant = findNodeById(node->getChildren(), id))
                {
                    return descendant;
                }
            }
            return nullptr;
        }

        Common::SharedPtr<DataNode> findNodeByName(
            const Common::DynamicArray<Common::SharedPtr<TreeNode>>& nodes,
            const Common::String& name)
        {
            for (const auto& node : nodes)
            {
                if (node->getName() == name)
                {
                    return Common::dynamicSharedPtrCast<DataNode>(node);
                }
                if (auto descendant = findNodeByName(node->getChildren(), name))
                {
                    return descendant;
                }
            }
            return nullptr;
        }

        bool containsId(const Common::DynamicArray<Common::SharedPtr<TreeNode>>& nodes,
                        DataNodeId id, const DataNode* excluded)
        {
            for (const auto& node : nodes)
            {
                if (node.get() != excluded && node->getId() == id)
                {
                    return true;
                }
                if (containsId(node->getChildren(), id, excluded))
                {
                    return true;
                }
            }
            return false;
        }

        bool containsName(const Common::DynamicArray<Common::SharedPtr<TreeNode>>& nodes,
                          const Common::String& name, const DataNode* excluded)
        {
            for (const auto& node : nodes)
            {
                if (node.get() != excluded && node->getName() == name)
                {
                    return true;
                }
                if (containsName(node->getChildren(), name, excluded))
                {
                    return true;
                }
            }
            return false;
        }

        Common::SharedPtr<DataNode> removeNodeFrom(
            Common::DynamicArray<Common::SharedPtr<TreeNode>>& nodes, DataNodeId id)
        {
            Common::DynamicArray<Common::SharedPtr<TreeNode>> remaining;
            Common::SharedPtr<DataNode> removed;
            for (const auto& node : nodes)
            {
                if (node->getId() == id)
                {
                    removed = Common::dynamicSharedPtrCast<DataNode>(node);
                }
                else
                {
                    remaining.pushBack(node);
                }
            }
            if (removed)
            {
                nodes = Common::move(remaining);
            }
            return removed;
        }
    } // namespace

    DataTree::DataTree(const Common::String& name) : TreeNode(0, name)
    {
    }

    Common::Return DataTree::addNode(const Common::SharedPtr<DataNode>& node, DataNodeId parentId)
    {
        if (!node)
        {
            return Common::Return::BadInput;
        }

        std::lock_guard<std::mutex> lock(_nodesMutex);
        if (node->getParent() != nullptr || node->getId() == 0 || containsId(node->getId()) ||
            containsName(node->getName()) || !node->getChildren().empty())
        {
            return Common::Return::Failed;
        }

        Common::DynamicArray<Common::SharedPtr<TreeNode>>* siblings = &_children;
        TreeNode* parent = this;
        if (parentId != 0)
        {
            auto parentNode = findNodeById(parentId);
            if (!parentNode)
            {
                return Common::Return::BadInput;
            }
            parent = parentNode.get();
            siblings = &parent->_children;
        }

        {
            std::lock_guard<std::mutex> parentLock(parent->_treeNodeMutex);
            siblings->pushBack(node);
        }
        {
            std::lock_guard<std::mutex> nodeLock(node->_treeNodeMutex);
            node->_parent = parent;
        }
        return Common::Return::Succeed;
    }

    Common::SharedPtr<DataNode> DataTree::findNode(DataNodeId id) const
    {
        std::lock_guard<std::mutex> lock(_nodesMutex);
        return findNodeById(id);
    }

    Common::SharedPtr<DataNode> DataTree::findNode(const Common::String& name) const
    {
        std::lock_guard<std::mutex> lock(_nodesMutex);
        return findNodeByName(name);
    }

    Common::SharedPtr<DataNode> DataTree::removeNode(DataNodeId id)
    {
        std::lock_guard<std::mutex> lock(_nodesMutex);
        auto node = findNodeById(id);
        if (!node)
        {
            return nullptr;
        }

        auto* parent = node->getParent();
        Common::SharedPtr<DataNode> removed;
        {
            std::lock_guard<std::mutex> parentLock(parent->_treeNodeMutex);
            removed = removeNodeFrom(parent->_children, id);
        }
        {
            std::lock_guard<std::mutex> nodeLock(removed->_treeNodeMutex);
            removed->_parent = nullptr;
        }
        return removed;
    }

    Common::Return DataTree::updateNode(DataNodeId id, const Common::SharedPtr<DataNode>& node)
    {
        if (!node || node->getId() != id)
        {
            return Common::Return::BadInput;
        }

        std::lock_guard<std::mutex> lock(_nodesMutex);
        auto existing = findNodeById(id);
        if (!existing)
        {
            return Common::Return::Failed;
        }
        if (existing == node)
        {
            return Common::Return::Succeed;
        }
        if (node->getParent() != nullptr || containsName(node->getName(), existing.get()))
        {
            return Common::Return::Failed;
        }

        auto children = node->getChildren();
        if (!children.empty())
        {
            return Common::Return::Failed;
        }

        TreeNode* parent = existing->getParent();
        {
            std::lock_guard<std::mutex> parentLock(parent->_treeNodeMutex);
            Common::DynamicArray<Common::SharedPtr<TreeNode>> updated;
            for (const auto& sibling : parent->_children)
            {
                if (sibling.get() == existing.get())
                {
                    updated.pushBack(node);
                }
                else
                {
                    updated.pushBack(sibling);
                }
            }
            parent->_children = Common::move(updated);
        }

        {
            std::scoped_lock nodeLocks(node->_treeNodeMutex, existing->_treeNodeMutex);
            node->_parent = parent;
            node->_children = Common::move(existing->_children);
            existing->_parent = nullptr;
        }
        for (const auto& child : node->getChildren())
        {
            std::lock_guard<std::mutex> childLock(child->_treeNodeMutex);
            child->_parent = node.get();
        }
        return Common::Return::Succeed;
    }

    Common::DynamicArray<Common::SharedPtr<DataNode>> DataTree::getDataNodes() const
    {
        std::lock_guard<std::mutex> lock(_nodesMutex);
        Common::DynamicArray<Common::SharedPtr<DataNode>> nodes;
        for (const auto& node : _children)
        {
            nodes.pushBack(Common::dynamicSharedPtrCast<DataNode>(node));
        }
        return nodes;
    }

    Common::SharedPtr<DataNode> DataTree::findNodeById(DataNodeId id) const
    {
        return EgLab::Platform::findNodeById(_children, id);
    }

    Common::SharedPtr<DataNode> DataTree::findNodeByName(const Common::String& name) const
    {
        return EgLab::Platform::findNodeByName(_children, name);
    }

    bool DataTree::containsId(DataNodeId id, const DataNode* excluded) const
    {
        return id == 0 || EgLab::Platform::containsId(_children, id, excluded);
    }

    bool DataTree::containsName(const Common::String& name, const DataNode* excluded) const
    {
        return name == getName() || EgLab::Platform::containsName(_children, name, excluded);
    }

    DataTree::~DataTree()
    {
    }
} // namespace EgLab::Platform
