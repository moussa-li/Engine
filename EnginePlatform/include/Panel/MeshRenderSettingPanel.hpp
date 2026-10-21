#pragma once

#include "Panel/IPanel.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    class MeshRenderSettingPanel : public IPanel
    {
    public:
        ~MeshRenderSettingPanel() override;

        Common::String getPanelName() override
        {
            return "MeshRenderSettings";
        }

        void render() override;

    private:
        bool _showNodeInfo{false};
        bool _showElementInfo{false};
        bool _showElementEdgeInfo{false};
        bool _showElementFaceInfo{false};
        float _highlightColor[4]{1.0f, 0.75f, 0.7f, 0.3f};
        PickInfoSubscriptionId _pickInfoSubscriptionId{0};
        bool _hasPickedInfo{false};
        PickedMeshInfo _pickedInfo;
    };
} // namespace EgLab::Platform