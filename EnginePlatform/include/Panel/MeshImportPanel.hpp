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

    protected:
        const char* getWindowTitle() const override
        {
            return "Mesh Import";
        }

        bool usesWindowFrame() const override
        {
            return false;
        }

        void renderImpl() override;
    };
} // namespace EgLab::Platform