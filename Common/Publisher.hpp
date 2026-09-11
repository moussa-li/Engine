#pragma once

#include <stddef.h>

#include <type_traits>

#include "Common/CommonAPI.hpp"
#include "Common/Protocol.hpp"

namespace EgLab::Common
{
    class CommonAPI Publisher
    {
    public:
        Publisher(MemoryQueue& queue);

        template <typename T>
        bool publish(uint32_t eventId, const T& data)
        {
            static_assert(std::is_trivially_copyable<T>::value,
                          "Data type must be trivially copyable for shared memory queue.");

            return _queue.push(static_cast<int32_t>(eventId), &data, sizeof(T));
        }

    private:
        MemoryQueue& _queue;
    };
} // namespace EgLab::Common