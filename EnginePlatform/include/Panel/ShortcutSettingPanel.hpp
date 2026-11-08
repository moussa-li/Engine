#pragma once

#include "Command/CommandCatalog.hpp"
#include "Panel/IPanel.hpp"

namespace EgLab::Platform
{
    // Shortcut settings panel: lists every command of the catalog, supports recording and
    // clearing chords, and can run a command directly (parameterised ones open the wizard).
    class ShortcutSettingPanel : public IPanel
    {
    public:
        ShortcutSettingPanel();
        ~ShortcutSettingPanel();

        virtual Common::String getPanelName() override
        {
            return kShortcutSettingPanelName;
        }

    protected:
        const char* getWindowTitle() const override
        {
            return "Shortcut Settings";
        }

        void configureWindow() const override;

        void renderImpl() override;
    };
} // namespace EgLab::Platform
