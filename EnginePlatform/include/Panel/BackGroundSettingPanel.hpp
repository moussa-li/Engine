#pragma once

#include "Panel/IPanel.hpp"

namespace EgLab::Platform
{

    class BackGroundSettingPanel : public IPanel
    {
    public:
        BackGroundSettingPanel();

        ~BackGroundSettingPanel();

        virtual Common::String getPanelName() override
        {
            return "BackGroundSetting";
        }

    protected:
        const char* getWindowTitle() const override
        {
            return "BackGround Settings";
        }

        void configureWindow() const override;

        void renderImpl() override;
    };

} // namespace EgLab::Platform
