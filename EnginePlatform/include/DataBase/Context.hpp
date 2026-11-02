#pragma once

#include "Common/SharedPtr.hpp"
#include "Core/Application.hpp"
#include "DataBase/DataNode.hpp"
#include "DataBase/DataTree.hpp"
#include "Panel/PanelManager.hpp"

namespace EgLab::Platform
{
    class Application;
    class Context : public Common::Singleton<Context>
    {
    public:
        Context();

        ~Context();

        inline PanelManager& getPanelManager()
        {
            return _panelManager;
        }

        inline Application& getApplication()
        {
            return _application;
        }

        inline DataTree& getDataTree()
        {
            return _dataTree;
        }

        Common::Return addDataNode(const Common::SharedPtr<DataNode>& node);
        Common::SharedPtr<DataNode> getDataNode(DataNodeId id) const;
        Common::SharedPtr<DataNode> removeDataNode(DataNodeId id);

    protected:
        friend class Common::Singleton<Context>;

    private:
        DataTree _dataTree{"Context"};
        PanelManager _panelManager;

        Application _application;
    };
} // namespace EgLab::Platform