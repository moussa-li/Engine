#pragma once

#include <mutex>

#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/String.hpp"
#include "DataBase/DataNode.hpp"

namespace EgLab::Platform
{
    class DataTree : public TreeNode
    {
    public:
        explicit DataTree(const Common::String& name);
        DataTree(const DataTree&) = delete;
        DataTree& operator=(const DataTree&) = delete;

        Common::Return addNode(const Common::SharedPtr<DataNode>& node, DataNodeId parentId = 0);
        Common::SharedPtr<DataNode> findNode(DataNodeId id) const;
        Common::SharedPtr<DataNode> findNode(const Common::String& name) const;
        Common::SharedPtr<DataNode> removeNode(DataNodeId id);
        Common::Return updateNode(DataNodeId id, const Common::SharedPtr<DataNode>& node);

        Common::DynamicArray<Common::SharedPtr<DataNode>> getDataNodes() const;
        virtual ~DataTree();

    private:
        Common::SharedPtr<DataNode> findNodeById(DataNodeId id) const;
        Common::SharedPtr<DataNode> findNodeByName(const Common::String& name) const;
        bool containsId(DataNodeId id, const DataNode* excluded = nullptr) const;
        bool containsName(const Common::String& name, const DataNode* excluded = nullptr) const;

        mutable std::mutex _nodesMutex;
    };
} // namespace EgLab::Platform
