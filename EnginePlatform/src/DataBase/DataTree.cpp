#include "DataBase/DataTree.hpp"

#include <mutex>

namespace EgLab::Platform
{
    DataTree::DataTree(const Common::String& name) : _name(name)
    {
    }

    Common::Return DataTree::addNode(const Common::SharedPtr<DataNode>& node)
    {
        if (!node)
        {
            return Common::Return::BadInput;
        }

        std::lock_guard<std::mutex> lock(_nodesMutex);
        for (const auto& current : _dataNodes)
        {
            if (current->getId() == node->getId())
            {
                return Common::Return::Failed;
            }
        }

        _dataNodes.pushBack(node);
        return Common::Return::Succeed;
    }

    Common::SharedPtr<DataNode> DataTree::findNode(DataNodeId id) const
    {
        std::lock_guard<std::mutex> lock(_nodesMutex);
        for (const auto& node : _dataNodes)
        {
            if (node->getId() == id)
            {
                return node;
            }
        }
        return nullptr;
    }

    Common::SharedPtr<DataNode> DataTree::removeNode(DataNodeId id)
    {
        std::lock_guard<std::mutex> lock(_nodesMutex);
        Common::DynamicArray<Common::SharedPtr<DataNode>> remaining;
        Common::SharedPtr<DataNode> removed;
        for (const auto& node : _dataNodes)
        {
            if (node->getId() == id)
            {
                removed = node;
            }
            else
            {
                remaining.pushBack(node);
            }
        }

        if (removed)
        {
            _dataNodes = Common::move(remaining);
        }
        return removed;
    }

    const Common::String& DataTree::getName() const
    {
        return _name;
    }

    Common::DynamicArray<Common::SharedPtr<DataNode>> DataTree::getDataNodes() const
    {
        std::lock_guard<std::mutex> lock(_nodesMutex);
        return _dataNodes;
    }

    DataTree::~DataTree()
    {
    }
} // namespace EgLab::Platform
