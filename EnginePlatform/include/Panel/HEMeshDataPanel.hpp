#pragma once

#include "Common/SharedPtr.hpp"
#include "DataBase/DataNode.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "Panel/IPanel.hpp"

namespace EgLab::Platform
{
    class DataTree;
    class MeshData;

    class HEMeshDataPanel : public IPanel
    {
    public:
        Common::String getPanelName() override
        {
            return "HEMesh";
        }

        void render() override;

    private:
        void renderNode(DataTree& node);
        void renderDataNode(DataNode& node);
        void renderMeshNode(MeshData& meshData);

        Common::SharedPtr<ME::Mesh> _selectedMesh;
    };
} // namespace EgLab::Platform