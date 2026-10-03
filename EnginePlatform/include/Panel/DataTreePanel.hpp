#pragma once

#include "Panel/IPanel.hpp"

namespace EgLab::Platform
{
    class DataTreePanel : public IPanel
    {
    public:
        Common::String getPanelName() override
        {
            return "DataTree";
        }

        void render() override;
    };
} // namespace EgLab::Platform
