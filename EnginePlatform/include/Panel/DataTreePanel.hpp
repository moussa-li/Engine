#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
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

        void render() override;

    private:
        bool renderNode(DataTree& node);

        bool renderMeshNode(MeshData& meshData);

        bool renderCommonNode(DataTree& node, bool isSelected = false);

    private:
        Common::DynamicArray<Common::SharedPtr<ME::Mesh>> _selectedMeshes;
    };
} // namespace EgLab::Platform
