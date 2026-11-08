#include "Panel/CommandParamWizardPanel.hpp"

#include "Command/CommandLauncher.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    CommandParamWizardPanel::CommandParamWizardPanel()
    {
    }

    CommandParamWizardPanel::~CommandParamWizardPanel()
    {
    }

    void CommandParamWizardPanel::configureWindow() const
    {
        ImGui::SetNextWindowSize(ImVec2(420.0f, 200.0f), ImGuiCond_FirstUseEver);
    }

    void CommandParamWizardPanel::syncTextBuffer(size_t step)
    {
        const Common::String current = CommandLauncher::instance().getDraftString(step);
        const size_t capacity = sizeof(_textBuffer) - 1;
        const size_t length = current.size() < capacity ? current.size() : capacity;
        const char* source = current.c_str();
        for (size_t i = 0; i < length; ++i)
        {
            _textBuffer[i] = source[i];
        }
        _textBuffer[length] = '\0';
    }

    void CommandParamWizardPanel::renderImpl()
    {
        CommandLauncher& launcher = CommandLauncher::instance();
        const CommandDesc* command = launcher.draftDesc();

        if (command == nullptr)
        {
            ImGui::TextDisabled("No command is waiting for parameters.");
            ImGui::TextDisabled("Trigger a command with parameters from the shortcut settings.");
            return;
        }

        // A parameterised command should always have at least one field; keep a fallback path.
        if (command->fieldCount == 0)
        {
            ImGui::Text("%s", command->displayName);
            ImGui::Separator();
            if (ImGui::Button("Run", ImVec2(100.0f, 0.0f)))
            {
                launcher.submitDraft();
                close();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(100.0f, 0.0f)))
            {
                launcher.discardDraft();
                close();
            }
            return;
        }

        const size_t step = launcher.draftStep() < command->fieldCount ? launcher.draftStep()
                                                                       : command->fieldCount - 1;
        const ParamFieldDesc& field = command->fields[step];

        ImGui::Text("%s", command->displayName);
        ImGui::TextDisabled("Step %zu of %zu", step + 1, command->fieldCount);
        ImGui::Separator();

        ImGui::TextUnformatted(field.label);

        if (field.type == ParamFieldType::String)
        {
            if (_syncedCommand != command->eventId || _syncedStep != step ||
                _syncedRevision != launcher.draftRevision())
            {
                _syncedCommand = command->eventId;
                _syncedStep = step;
                _syncedRevision = launcher.draftRevision();
                syncTextBuffer(step);
            }

            const size_t bufferSize = (field.capacity > 0 && field.capacity <= sizeof(_textBuffer))
                                          ? field.capacity
                                          : sizeof(_textBuffer);
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::InputTextWithHint("##paramValue", field.hint != nullptr ? field.hint : "",
                                         _textBuffer, static_cast<int>(bufferSize)))
            {
                launcher.setDraftString(step, _textBuffer);
            }
        }
        else if (field.type == ParamFieldType::Bool)
        {
            bool value = launcher.getDraftUInt(step) != 0;
            if (ImGui::Checkbox("##paramValue", &value))
            {
                launcher.setDraftUInt(step, value ? 1u : 0u);
            }
        }
        else
        {
            ImGui::SetNextItemWidth(-1.0f);
            if (field.type == ParamFieldType::UInt32)
            {
                uint32_t value = static_cast<uint32_t>(launcher.getDraftUInt(step));
                if (ImGui::InputScalar("##paramValue", ImGuiDataType_U32, &value))
                {
                    launcher.setDraftUInt(step, value);
                }
            }
            else
            {
                uint64_t value = launcher.getDraftUInt(step);
                if (ImGui::InputScalar("##paramValue", ImGuiDataType_U64, &value))
                {
                    launcher.setDraftUInt(step, value);
                }
            }
        }

        ImGui::Separator();

        if (step > 0)
        {
            if (ImGui::Button("Back", ImVec2(100.0f, 0.0f)))
            {
                launcher.setDraftStep(step - 1);
            }
            ImGui::SameLine();
        }

        const bool lastStep = step + 1 >= command->fieldCount;
        if (lastStep)
        {
            if (ImGui::Button("Run", ImVec2(100.0f, 0.0f)))
            {
                launcher.submitDraft();
                close();
                return;
            }
        }
        else
        {
            if (ImGui::Button("Next", ImVec2(100.0f, 0.0f)))
            {
                launcher.setDraftStep(step + 1);
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100.0f, 0.0f)))
        {
            launcher.discardDraft();
            close();
        }
    }

    void CommandParamWizardPanel::afterRenderFrame()
    {
        // Closing with the window X button must also drop the draft so nothing lingers.
        if (!isShow())
        {
            CommandLauncher::instance().discardDraft();
        }
    }
} // namespace EgLab::Platform
