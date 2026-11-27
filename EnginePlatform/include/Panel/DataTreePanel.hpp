#pragma once

#include "Common/DynamicArray.hpp"
#include "DataBase/DataNode.hpp"
#include "Panel/GeometryFeatureBrowser.hpp"
#include "Panel/IPanel.hpp"
#include "Panel/MeshFeatureBrowser.hpp"

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

        // Called by close() (Escape, hidePanel) before the panel hides.
        void abortClose() override;

        // Catches the window X button, which hides the panel without calling close().
        void afterRenderFrame() override;

    private:
        bool renderNode(DataTree& node);

        void renderDataNode(DataNode& node);

        bool renderMeshNode(MeshData& meshData);

        // Shows or hides every element/node of one mesh through the visibility commands.
        void setMeshVisible(MeshData& meshData, bool visible);

        // Highlights the mesh in the viewport and moves the camera to it.
        void locateMesh(MeshData& meshData);

        // Clears the highlight requested by locateMesh when the panel is hidden.
        void clearLocateHighlight();

    private:
        Common::DynamicArray<DataNodeId> _selectedDataNodeIds;

        // True while a Locate highlight is active, so it can be cleared when the panel closes.
        bool _highlightActive{false};

        // Feature browser opened by double-clicking a GeometryData node.
        GeometryFeatureBrowser _featureBrowser;

        // Mesh browser opened by double-clicking a plain MeshData node.
        MeshFeatureBrowser _meshBrowser;
    };
} // namespace EgLab::Platform
