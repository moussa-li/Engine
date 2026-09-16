#pragma once

#include "Panel/IPanel.hpp"

namespace EgLab::Platform
{
    class MeshImportPanel : public IPanel
    {
    public:
        MeshImportPanel();

        virtual Common::String getPanelName() override
        {
            return "MeshImport";
        }

        virtual void abortShow() override;

        virtual void render() override;
    };
} // namespace EgLab::Platform