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

        virtual void render() override;
    };

} // namespace EgLab::Platform
