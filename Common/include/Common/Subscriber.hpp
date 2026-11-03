#pragma once

#include <type_traits>

#include "Common/CommonAPI.hpp"
#include "Common/HashSet.hpp"
#include "Common/Protocol.hpp"

namespace EgLab::Common
{
    class CommonAPI Subscriber
    {
    public:
        Subscriber(MemoryQueue& queue);

        ~Subscriber();

        void subscrib(uint32_t eventId);

        template <typename T>
        bool tryReceive(uint32_t& eventId, T& outData)
        {
            static_assert(std::is_trivially_copyable<T>::value,
                          "Data type must be trivially copyable for shared memory queue.");

            uint32_t outChannelId = 0;
            uint32_t actualSize = 0;

            if (_tempBuffer.size() < sizeof(T)) _tempBuffer.resize(sizeof(T));

            if (_queue.pop(outChannelId, _tempBuffer.data(),
                           static_cast<uint32_t>(_tempBuffer.size()), actualSize))
            {
                // 将底层弹出的 int32_t 转回 EventId 进行比较
                if (_targetEventId.find(static_cast<int32_t>(outChannelId)) !=
                        _targetEventId.end() &&
                    actualSize == sizeof(T))
                {
                    eventId = outChannelId;
                    std::memcpy(&outData, _tempBuffer.data(), sizeof(T));
                    return true;
                }
            }
            return false;
        }

        bool tryReceiveDynamic(uint32_t& eventId, DynamicArray<uint8_t>& outData);

    private:
        MemoryQueue& _queue;
        HashSet<uint32_t> _targetEventId;
        DynamicArray<uint8_t> _tempBuffer;
    };
} // namespace EgLab::Common