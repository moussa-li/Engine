#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Message.hpp"
#include "Panel/IPanel.hpp"

namespace EgLab::Platform
{
    // 完整信息面板：展示自启动以来发送过的所有消息条目。
    class MessageLogPanel : public IPanel
    {
    public:
        MessageLogPanel();
        ~MessageLogPanel();

        Common::String getPanelName() override
        {
            return "MessageLog";
        }

        void pushMessage(const Common::Message& message);

    protected:
        const char* getWindowTitle() const override
        {
            return "Message Log";
        }

        void configureWindow() const override;
        void renderImpl() override;

    private:
        Common::DynamicArray<Common::Message> _messages;
        uint64_t _nextSequence{1};
    };
} // namespace EgLab::Platform
