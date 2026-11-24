#pragma once

#include "Common/SharedPtr.hpp"
#include "Common/String.hpp"
#include "DataBase/GeometryData.hpp"
#include "Panel/IPanel.hpp"
#include "RenderEngine/Core/PickManager.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    /// Shows geometry feature info for the face under the cursor, mirroring the
    /// node/element info tooltip in MeshRenderSettingPanel.
    class GeometryInfoPanel : public IPanel
    {
    public:
        GeometryInfoPanel();
        ~GeometryInfoPanel() override;

        Common::String getPanelName() override
        {
            return Common::String("GeometryInfo");
        }

    protected:
        const char* getWindowTitle() const override
        {
            return "Geometry Info";
        }

        void configureWindow() const override;
        void renderImpl() override;
        void afterRenderFrame() override;
        void abortClose() override;

    private:
        void syncPickRequest();
        void releasePickRequest();

        void updatePickedInfo();
        static Common::String featureTypeName(uint32_t type);
        static Common::String geometryTypeName(uint32_t type);

    private:
        bool _showHoverInfo{true};
        PickInfoSubscriptionId _pickInfoSubscriptionId{0};
        bool _hasPickedInfo{false};
        PickedMeshInfo _pickedInfo;
        GeometryFeatureInfo _featureInfo;
        Common::String _hoverStatus;
        RE::PickManager::PickToken _facePickToken{RE::PickManager::kInvalidToken};
    };
} // namespace EgLab::Platform
