#pragma once

#include "Panel/IPanel.hpp"

namespace EgLab::Platform
{
    class DataTree;

    class DataTreePanel : public IPanel
    {
    public:
        Common::String getPanelName() override
        {
            return "DataTree";
        }

        void render() override;

    private:
        DataTree* _selectedNode{nullptr};
    };
} // namespace EgLab::Platform
