#pragma once

#include "Panel/IPanel.hpp"

namespace EgLab::Platform
{
    class CameraSettingPanel : public IPanel
    {
    public:
        CameraSettingPanel();
        ~CameraSettingPanel();

        virtual Common::String getPanelName() override
        {
            return "CameraSetting";
        }

        virtual void render() override;
    };
} // namespace EgLab::Platform