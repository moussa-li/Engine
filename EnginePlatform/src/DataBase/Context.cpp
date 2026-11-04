#include "DataBase/Context.hpp"

namespace EgLab::Platform
{
    Context::Context()
    {
    }

    Context::~Context()
    {
    }

    Common::Return Context::addDataNode(const Common::SharedPtr<DataNode>& node)
    {
        return _dataTree.addNode(node);
    }

    Common::SharedPtr<DataNode> Context::getDataNode(DataNodeId id) const
    {
        return _dataTree.findNode(id);
    }

    Common::SharedPtr<DataNode> Context::getDataNode(const Common::String& name) const
    {
        return _dataTree.findNode(name);
    }

    Common::SharedPtr<DataNode> Context::removeDataNode(DataNodeId id)
    {
        return _dataTree.removeNode(id);
    }

    Common::Return Context::updateDataNode(DataNodeId id, const Common::SharedPtr<DataNode>& node)
    {
        return _dataTree.updateNode(id, node);
    }
} // namespace EgLab::Platform
