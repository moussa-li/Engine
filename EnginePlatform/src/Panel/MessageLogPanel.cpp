#include "Panel/MessageLogPanel.hpp"

#include "imgui.h"

namespace EgLab::Platform
{
    namespace
    {
        // 完整信息面板最多保留的消息条数，避免无限增长。
        constexpr size_t kMaxLogMessages = 512;

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
                return {"INFO", ImVec4(0.65f, 0.65f, 0.65f, 1.0f)};
            case Common::MessageLevel::Warning:
                return {"WARNING", ImVec4(1.0f, 0.78f, 0.25f, 1.0f)};
            case Common::MessageLevel::Error:
                return {"ERROR", ImVec4(1.0f, 0.35f, 0.35f, 1.0f)};
            case Common::MessageLevel::Fatal:
                return {"FATAL", ImVec4(1.0f, 0.25f, 0.60f, 1.0f)};
            default:
                return {"INFO", ImVec4(0.65f, 0.65f, 0.65f, 1.0f)};
            }
        }
    } // namespace

    MessageLogPanel::MessageLogPanel()
    {
    }

    MessageLogPanel::~MessageLogPanel()
    {
    }

    void MessageLogPanel::pushMessage(const Common::Message& message)
    {
        Common::Message stamped = message;
        stamped.sequence = _nextSequence++;

        if (_messages.size() >= kMaxLogMessages)
        {
            // 丢弃最旧的一条：整体左移后弹出末尾。
            for (size_t index = 0; index + 1 < _messages.size(); ++index)
            {
                _messages[index] = _messages[index + 1];
            }
            _messages.popBack();
        }
        _messages.pushBack(stamped);
    }

    void MessageLogPanel::configureWindow() const
    {
        ImGui::SetNextWindowSize(ImVec2(520, 360), ImGuiCond_FirstUseEver);
    }

    void MessageLogPanel::renderImpl()
    {
        if (ImGui::Button("Clear"))
        {
            _messages.clear();
        }
        ImGui::SameLine();
        ImGui::Text("Total %d", static_cast<int>(_messages.size()));
        ImGui::Separator();

        ImGui::BeginChild("MessageLogScroll", ImVec2(0, 0), false,
                          ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto& message : _messages)
        {
            const LevelStyle style = levelStyle(static_cast<Common::MessageLevel>(message.level));
            ImGui::TextColored(style.color, "[%llu][%s] %s",
                               static_cast<unsigned long long>(message.sequence), style.tag,
                               message.text);
        }
        ImGui::EndChild();
    }
} // namespace EgLab::Platform
