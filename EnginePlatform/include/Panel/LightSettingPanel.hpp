#pragma once

#include "Panel/IPanel.hpp"

namespace EgLab::Platform
{
    class LightSettingPanel : public IPanel
    {
    public:
        Common::String getPanelName() override
        {
            return "LightSetting";
        }

    protected:
        const char* getWindowTitle() const override
        {
            return "Light Settings";
        }

        void configureWindow() const override;

        void renderImpl() override;
    };
} // namespace EgLab::Platform