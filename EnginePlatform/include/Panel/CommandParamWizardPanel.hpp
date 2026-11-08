#pragma once

#include <cstddef>

#include "Command/CommandCatalog.hpp"
#include "Panel/IPanel.hpp"

namespace EgLab::Platform
{
    // Parameter wizard panel: walks the CommandParam field description of the catalog one step
    // at a time, and hands the completed draft to CommandLauncher for dispatch.
    class CommandParamWizardPanel : public IPanel
    {
    public:
        CommandParamWizardPanel();
        ~CommandParamWizardPanel();

        virtual Common::String getPanelName() override
        {
            return kCommandParamWizardPanelName;
        }

    protected:
        const char* getWindowTitle() const override
        {
            return "Command Parameters";
        }

        void configureWindow() const override;

        void renderImpl() override;

        void afterRenderFrame() override;

    private:
        void syncTextBuffer(size_t step);

        char _textBuffer[256]{};
        EventId _syncedCommand{EventId::Unknow};
        size_t _syncedStep{static_cast<size_t>(-1)};
        uint64_t _syncedRevision{0};
    };
} // namespace EgLab::Platform
