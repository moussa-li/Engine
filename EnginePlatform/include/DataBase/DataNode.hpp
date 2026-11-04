#pragma once

#include <cstdint>
#include <mutex>

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/String.hpp"

namespace EgLab::Platform
{
    using DataNodeId = uint64_t;

    class TreeNode
    {
    public:
        TreeNode(DataNodeId id, const Common::String& name) : _id(id), _name(name)
        {
        }

        virtual ~TreeNode()
        {
            for (const auto& child : _children)
            {
                std::lock_guard<std::mutex> lock(child->_treeNodeMutex);
                if (child->_parent == this)
                {
                    child->_parent = nullptr;
                }
            }
        }

        DataNodeId getId() const
        {
            return _id;
        }

        const Common::String& getName() const
        {
            return _name;
        }

        TreeNode* getParent() const
        {
            std::lock_guard<std::mutex> lock(_treeNodeMutex);
            return _parent;
        }

        Common::DynamicArray<Common::SharedPtr<TreeNode>> getChildren() const
        {
            std::lock_guard<std::mutex> lock(_treeNodeMutex);
            return _children;
        }

    private:
        DataNodeId _id;
        Common::String _name;
        mutable std::mutex _treeNodeMutex;
        TreeNode* _parent{nullptr};
        Common::DynamicArray<Common::SharedPtr<TreeNode>> _children;

        friend class DataTree;
    };

    class DataNode : public TreeNode
    {
    public:
        using TreeNode::TreeNode;
        ~DataNode() override = default;
    };
} // namespace EgLab::Platform