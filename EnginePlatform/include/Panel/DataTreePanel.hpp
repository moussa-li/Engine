#pragma once

#include "Common/DynamicArray.hpp"
#include "DataBase/DataNode.hpp"
#include "Panel/IPanel.hpp"

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

        // Sends the current selection to CommandManager as a Highlight command.
        void commitHighlightSelection();

        // Clears the selection and removes the highlight when the panel is closed.
        void clearSelectionAndHighlight();

    private:
        Common::DynamicArray<DataNodeId> _selectedDataNodeIds;
    };
} // namespace EgLab::Platform
