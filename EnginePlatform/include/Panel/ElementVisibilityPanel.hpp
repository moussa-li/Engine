#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/String.hpp"
#include "DataBase/DataNode.hpp"
#include "Panel/IPanel.hpp"

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
        void syncPlanePreview();

        Common::DynamicArray<MeshEntry> _meshEntries;
        int _selectedMeshIndex{-1};
        float _normal[3]{0.0f, 0.0f, 1.0f};
        float _referencePoint[3]{0.0f, 0.0f, 0.0f};
        float _referenceMin{0.0f};
        float _referenceMax{0.0f};
        float _boundsMin[3]{0.0f, 0.0f, 0.0f};
        float _boundsMax[3]{0.0f, 0.0f, 0.0f};
        float _planeExtent{10.0f};
        bool _autoApply{true};
        DataNodeId _boundsNodeId{0};
        Common::DynamicArray<uint32_t> _positiveElementIds;
        Common::DynamicArray<uint32_t> _negativeElementIds;

        float _lastNormal[3]{0.0f, 0.0f, -1.0f};
        float _lastPoint[3]{0.0f, 0.0f, 0.0f};
        float _lastExtent{0.0f};
        bool _planePreviewVisible{false};
    };
} // namespace EgLab::Platform
