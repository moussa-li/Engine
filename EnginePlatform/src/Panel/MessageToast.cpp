#include "Panel/MessageToast.hpp"

#include "imgui.h"

namespace EgLab::Platform
{
    namespace
    {
        struct LevelStyle
        {
            const char* tag;
            ImVec4 color;
        };

        LevelStyle levelStyle(Common::MessageLevel level)
        {
            switch (level)
            {
            case Common::MessageLevel::Info:
                return {"INFO", ImVec4(0.55f, 0.75f, 1.0f, 1.0f)};
            case Common::MessageLevel::Warning:
                return {"WARNING", ImVec4(1.0f, 0.78f, 0.25f, 1.0f)};
            case Common::MessageLevel::Error:
                return {"ERROR", ImVec4(1.0f, 0.35f, 0.35f, 1.0f)};
            case Common::MessageLevel::Fatal:
                return {"FATAL", ImVec4(1.0f, 0.25f, 0.60f, 1.0f)};
            default:
                return {"INFO", ImVec4(0.55f, 0.75f, 1.0f, 1.0f)};
            }
        }
    } // namespace

    MessageToast::MessageToast()
    {
    }

    MessageToast::~MessageToast()
    {
    }

    void MessageToast::pushMessage(const Common::Message& message)
    {
        _current = message;
        _hasMessage = true;
        _elapsed = 0.0f;
        _alpha = 1.0f;
        show();
    }

    void MessageToast::renderImpl()
    {
        if (!_hasMessage)
        {
            return;
        }

        ImGuiIO& io = ImGui::GetIO();
        _elapsed += io.DeltaTime;

        if (_elapsed < _holdDuration)
        {
            _alpha = 1.0f;
        }
        else
        {
            _alpha = 1.0f - (_elapsed - _holdDuration) / _fadeDuration;
        }

        if (_alpha <= 0.0f)
        {
            _hasMessage = false;
            close();
            return;
        }

        const ImVec2 displaySize = io.DisplaySize;
        const ImVec2 toastSize(360.0f, 92.0f);
        const ImVec2 toastPos(displaySize.x - toastSize.x - 16.0f,
                              displaySize.y - toastSize.y - 16.0f);

        ImGui::SetNextWindowPos(toastPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(toastSize, ImGuiCond_Always);

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, _alpha);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 10.0f));

        const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoSavedSettings |
                                       ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

        bool open = true;
        if (ImGui::Begin("MessageToast", &open, flags))
        {
            const LevelStyle style = levelStyle(static_cast<Common::MessageLevel>(_current.level));

            ImGui::TextColored(style.color, "[%s]", style.tag);

            ImGui::SameLine();
            ImGui::SetCursorPosX(toastSize.x - 12.0f - 20.0f);
            if (ImGui::SmallButton("x"))
            {
                _hasMessage = false;
                close();
            }

            ImGui::Separator();
            ImGui::TextWrapped("%s", _current.text);
        }
        ImGui::End();

        ImGui::PopStyleVar(4);
    }
} // namespace EgLab::Platform
