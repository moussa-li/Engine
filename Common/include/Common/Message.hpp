#pragma once

#include <cstdint>
#include <type_traits>

namespace EgLab::Common
{
    // 消息定级：INFO / WARNING / ERROR / FATAL。
    enum class MessageLevel : int32_t
    {
        Info = 0,
        Warning = 1,
        Error = 2,
        Fatal = 3
    };

    // 消息事件在 Publisher / Subscriber 队列中的通道号（Common 层事件 id）。
    constexpr int32_t kMessageEventId = 300001;

    // 单条消息文本最大长度（不含结尾 '\0'）。
    constexpr size_t kMessageTextCapacity = 512;

    // 可跨线程 / 跨进程传输的消息载体，必须保持 trivially copyable。
    struct Message
    {
        int32_t level{0};     // MessageLevel 的底层值
        uint64_t sequence{0}; // 单调递增的消息序号
        char text[kMessageTextCapacity]{};
    };

    static_assert(std::is_trivially_copyable<Message>::value,
                  "Message must be trivially copyable for the shared memory queue");
} // namespace EgLab::Common
