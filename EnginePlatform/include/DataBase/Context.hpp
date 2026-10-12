#pragma once

#include <mutex>

#include "Common/SharedPtr.hpp"
#include "Core/Application.hpp"
#include "DataBase/DataTree.hpp"
#include "Panel/PanelManager.hpp"

namespace EgLab::ME
{
    class Mesh;
}

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

        Common::Return addMeshData(const Common::SharedPtr<ME::Mesh>& mesh,
                                   const Common::String& name);
        Common::SharedPtr<ME::Mesh> removeMeshData(const ME::Mesh* mesh);

    protected:
        friend class Common::Singleton<Context>;

    private:
        std::mutex _dataTreeMutex;
        DataTree _dataTree{"Context"};
        PanelManager _panelManager;

        Application _application;
    };
} // namespace EgLab::Platform