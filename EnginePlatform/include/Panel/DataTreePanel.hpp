#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "DataBase/DataNode.hpp"
#include "Panel/IPanel.hpp"

namespace EgLab::ME
{
    class Mesh;
}

namespace EgLab::Platform
{
    class DataTree;
    class MeshData;

    class DataTreePanel : public IPanel
    {
    public:
        Common::String getPanelName() override
        {
            return "DataTree";
        }

        DataTreePanel();

        virtual ~DataTreePanel();

    protected:
        const char* getWindowTitle() const override
        {
            return "Data Tree";
        }

        void configureWindow() const override;

        void renderImpl() override;

    private:
        bool renderNode(DataTree& node);

        void renderDataNode(DataNode& node);

        bool renderMeshNode(MeshData& meshData);

    private:
        Common::DynamicArray<Common::SharedPtr<ME::Mesh>> _selectedMeshes;
        Common::DynamicArray<DataNodeId> _selectedDataNodeIds;
    };
} // namespace EgLab::Platform
