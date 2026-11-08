#include "Panel/ShortcutSettingPanel.hpp"

#include "Command/CommandCatalog.hpp"
#include "Command/CommandLauncher.hpp"
#include "Command/ShortcutManager.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    ShortcutSettingPanel::ShortcutSettingPanel()
    {
    }

    ShortcutSettingPanel::~ShortcutSettingPanel()
    {
    }

    void ShortcutSettingPanel::configureWindow() const
    {
        ImGui::SetNextWindowSize(ImVec2(560.0f, 460.0f), ImGuiCond_FirstUseEver);
    }

    void ShortcutSettingPanel::renderImpl()
    {
        ShortcutManager& shortcuts = ShortcutManager::instance();

        if (ImGui::Button("Reset to Defaults"))
        {
            shortcuts.resetToDefaults();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("Click Bind, then press a key combination (Esc cancels).");

        ImGui::Separator();

        size_t count = 0;
        const CommandDesc* commands = CommandCatalog::commands(count);

        bool hasCategory = false;
        Common::String currentCategory;
        for (size_t i = 0; i < count; ++i)
        {
            const CommandDesc& command = commands[i];

            if (!hasCategory || !(currentCategory == command.category))
            {
                hasCategory = true;
                currentCategory = Common::String(command.category);
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.55f, 0.75f, 1.0f, 1.0f), "%s", currentCategory.c_str());
                ImGui::Separator();
            }

            ImGui::PushID(static_cast<int>(command.eventId));

            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(command.displayName);
            if (command.needsPacket)
            {
                ImGui::SameLine();
                ImGui::TextDisabled("(needs params)");
            }

            ImGui::SameLine(230.0f);

            const bool capturing = shortcuts.captureTarget() == command.eventId;
            const Common::String chordText =
                capturing ? Common::String("Press keys...")
                          : (shortcuts.isBound(command.eventId)
                                 ? shortcuts.getBinding(command.eventId).toString()
                                 : Common::String("<unbound>"));

            if (capturing)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.42f, 0.12f, 1.0f));
            }
            ImGui::Button(chordText.c_str(), ImVec2(150.0f, 0.0f));
            if (capturing)
            {
                ImGui::PopStyleColor();
            }

            ImGui::SameLine();
            if (capturing)
            {
                if (ImGui::Button("Cancel"))
                {
                    shortcuts.cancelCapture();
                }
            }
            else
            {
                if (ImGui::Button("Bind"))
                {
                    shortcuts.beginCapture(command.eventId);
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Clear"))
            {
                shortcuts.unbind(command.eventId);
            }

            ImGui::SameLine();
            if (ImGui::Button("Run"))
            {
                CommandLauncher::instance().invoke(command.eventId);
            }

            ImGui::PopID();
        }
    }
} // namespace EgLab::Platform
