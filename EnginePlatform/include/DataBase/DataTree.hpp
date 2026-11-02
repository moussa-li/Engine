#pragma once

#include <mutex>

#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/String.hpp"
#include "DataBase/DataNode.hpp"

namespace EgLab::Platform
{
    class DataTree
    {
    public:
        explicit DataTree(const Common::String& name);
        DataTree(const DataTree&) = delete;
        DataTree& operator=(const DataTree&) = delete;

        Common::Return addNode(const Common::SharedPtr<DataNode>& node);
        Common::SharedPtr<DataNode> findNode(DataNodeId id) const;
        Common::SharedPtr<DataNode> removeNode(DataNodeId id);

        const Common::String& getName() const;
        Common::DynamicArray<Common::SharedPtr<DataNode>> getDataNodes() const;
        virtual ~DataTree();

    private:
        Common::String _name;
        mutable std::mutex _nodesMutex;
        Common::DynamicArray<Common::SharedPtr<DataNode>> _dataNodes;
    };
} // namespace EgLab::Platform
