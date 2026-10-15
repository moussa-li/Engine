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
        PickInfoSubscriptionId _pickInfoSubscriptionId{0};
        bool _hasPickedInfo{false};
        PickedMeshInfo _pickedInfo;
    };
} // namespace EgLab::Platform