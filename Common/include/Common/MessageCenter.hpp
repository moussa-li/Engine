#pragma once

#include "Common/CommonAPI.hpp"
#include "Common/Message.hpp"
#include "Common/Protocol.hpp"
#include "Common/Publisher.hpp"
#include "Common/Singleton.hpp"
#include "Common/String.hpp"
#include "Common/Subscriber.hpp"

namespace EgLab::Common
{
    // 消息回调中枢：
    // - RenderEngine / MeshEngine 通过 emit 发送消息；
    // - EnginePlatform（UI 层）通过 setHandler 将处理函数塞入回调，
    //   并在每帧调用 poll 取出消息驱动 UI。
    class CommonAPI MessageCenter : public Singleton<MessageCenter>
    {
    public:
        using MessageHandler = void (*)(const Message&);

        // 业务引擎（RenderEngine / MeshEngine 等）调用，发送一条消息。
        void emit(MessageLevel level, const String& text);
        void emit(MessageLevel level, const char* text);

        // UI 层（EnginePlatform）注册消息处理回调。
        void setHandler(MessageHandler handler);

        // 取出队列中所有待处理消息并逐条回调给已注册的 handler。
        void poll();

    private:
        MessageCenter();
        ~MessageCenter() = default;

        friend class Singleton<MessageCenter>;

        MemoryQueue _queue{"Message Center"};
        Publisher _publisher{_queue};
        Subscriber _subscriber{_queue};
        MessageHandler _handler{nullptr};
    };
} // namespace EgLab::Common
