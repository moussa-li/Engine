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

    protected:
        const char* getWindowTitle() const override
        {
            return "Camera Setting";
        }

        void configureWindow() const override;

        void renderImpl() override;
    };
} // namespace EgLab::Platform