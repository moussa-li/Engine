#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/String.hpp"
#include "DataBase/DataNode.hpp"
#include "Panel/IPanel.hpp"
#include "Work/RenderWork.hpp"
#include "Work/SelectionModel.hpp"

namespace EgLab::ME
{
    class Mesh;
} // namespace EgLab::ME

namespace EgLab::Platform
{
    // Panel that hides mesh elements below a user defined reference plane. Elements whose
    // centroid lies on the negative side of the plane are hidden through ElementsVisibilityCmd.
    class ElementVisibilityPanel : public IPanel
    {
    public:
        ElementVisibilityPanel();
        ~ElementVisibilityPanel() override;

        Common::String getPanelName() override
        {
            return "ElementVisibility";
        }

    protected:
        const char* getWindowTitle() const override
        {
            return "Element Visibility";
        }

        void configureWindow() const override;
        void renderImpl() override;
        void afterRenderFrame() override;
        void abortShow() override;
        void abortClose() override;

    private:
        struct MeshEntry
        {
            DataNodeId nodeId{0};
            Common::String name;
        };

        void refreshMeshEntries();
        Common::SharedPtr<ME::Mesh> getSelectedMesh() const;
        void updateReferenceBounds(const Common::SharedPtr<ME::Mesh>& mesh, DataNodeId nodeId);
        void applyPlaneCut();
        void requestShowAll();
        void requestElementsVisibility(DataNodeId nodeId, uint32_t mode,
                                       const Common::String& ids);
        void requestNodeVisibility(DataNodeId nodeId, uint32_t mode, const Common::String& ids);
        // Refreshes the cached selection snapshot from the render-side selection model.
        void refreshSelectionSnapshot();
        void renderSelectionSection();
        void applySelectionVisibility(uint32_t elementMode, uint32_t nodeMode);
        void requestShowAllInScope();
        bool findMeshEntry(const Common::SharedPtr<ME::Mesh>& mesh, MeshEntry& entry) const;
        Common::SharedPtr<ME::Mesh> getMeshByEntryIndex(int entryIndex) const;
        // Rebuilds the temporary translucent quad that previews the cut plane. The previous
        // preview mesh is retired and the new one is handed to RenderWork.
        void updatePlanePreview();
        void hidePlanePreview();

        Common::DynamicArray<MeshEntry> _meshEntries;
        int _selectedMeshIndex{-1};
        float _normal[3]{0.0f, 0.0f, 1.0f};
        float _referencePoint[3]{0.0f, 0.0f, 0.0f};
        float _referenceMin{0.0f};
        float _referenceMax{0.0f};
        float _boundsMin[3]{0.0f, 0.0f, 0.0f};
        float _boundsMax[3]{0.0f, 0.0f, 0.0f};
        bool _autoApply{true};
        bool _showPreview{true};
        float _previewOpacity{0.25f};
        float _previewHalfExtent{1.0f};
        // Temporary mesh that renders the cut plane. Owned here so it can be replaced or
        // retired through RenderWork's regular mesh path.
        Common::SharedPtr<ME::Mesh> _previewMesh;
        DataNodeId _boundsNodeId{0};
        Common::DynamicArray<uint32_t> _positiveElementIds;
        Common::DynamicArray<uint32_t> _negativeElementIds;

        // ---- selection group ------------------------------------------------------------
        int _selectionScopeIndex{0};
        bool _selectionContainsOnly{true};
        SelectionChangedSubscriptionId _selectionInfoSubscriptionId{0};
        SelectionSnapshot _selectionSnapshot;
        SelectionChangedInfo _selectionCounts;
        bool _selectionSnapshotDirty{true};
    };
} // namespace EgLab::Platform
