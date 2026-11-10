#pragma once

#include "Common/Message.hpp"
#include "Panel/IPanel.hpp"

namespace EgLab::Platform
{
    // 及时信息面板：消息发出时在右下角弹出，完全显示一段时间后逐渐淡出，
    // 并提供一个关闭按键立即关闭。
    class MessageToast : public IPanel
    {
    public:
        MessageToast();
        ~MessageToast();

        Common::String getPanelName() override
        {
            return "MessageToast";
        }

        void pushMessage(const Common::Message& message);

    protected:
        bool usesWindowFrame() const override
        {
            return false;
        }

        const char* getWindowTitle() const override
        {
            return "Message Toast";
        }

        void renderImpl() override;

    private:
        Common::Message _current;
        bool _hasMessage{false};
        float _elapsed{0.0f};
        float _holdDuration{5.0f}; // 完全显示时长（秒）
        float _fadeDuration{1.0f}; // 淡出时长（秒）
        float _alpha{1.0f};
    };
} // namespace EgLab::Platform
