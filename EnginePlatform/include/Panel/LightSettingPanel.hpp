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

        void render() override;
    };
} // namespace EgLab::Platform