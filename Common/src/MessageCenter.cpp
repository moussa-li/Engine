#include "Common/MessageCenter.hpp"

#include <cstring>

namespace EgLab::Common
{
    MessageCenter::MessageCenter()
    {
        _subscriber.subscrib(static_cast<uint32_t>(kMessageEventId));
    }

    void MessageCenter::emit(MessageLevel level, const String& text)
    {
        emit(level, text.c_str());
    }

    void MessageCenter::emit(MessageLevel level, const char* text)
    {
        if (text == nullptr)
        {
            return;
        }

        Message message{};
        message.level = static_cast<int32_t>(level);

        const size_t textLength = std::strlen(text);
        const size_t copySize =
            textLength < kMessageTextCapacity ? textLength : kMessageTextCapacity - 1;
        std::memcpy(message.text, text, copySize);
        message.text[copySize] = '\0';

        _publisher.publish(static_cast<uint32_t>(kMessageEventId), message);
    }

    void MessageCenter::setHandler(MessageHandler handler)
    {
        _handler = handler;
    }

    void MessageCenter::poll()
    {
        if (_handler == nullptr)
        {
            return;
        }

        uint32_t eventId = 0;
        Message message{};
        while (_subscriber.tryReceive(eventId, message))
        {
            _handler(message);
        }
    }
} // namespace EgLab::Common
