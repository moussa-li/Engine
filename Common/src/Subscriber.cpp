#include "Subscriber.hpp"

namespace EgLab::Common
{
    Subscriber::Subscriber(MemoryQueue& queue) : _queue(queue)
    {
    }

    Subscriber::~Subscriber()
    {
    }

    void Subscriber::subscrib(uint32_t eventId)
    {
        _targetEventId.insert(eventId);
    }

    bool Subscriber::tryReceiveDynamic(uint32_t& eventId, DynamicArray<uint8_t>& outData)
    {
        uint32_t outChannelId = 0;
        if (_queue.pop(outChannelId, outData) &&
            _targetEventId.find(static_cast<int32_t>(outChannelId)) != _targetEventId.end())
        {
            eventId = outChannelId;
            return true;
        }
        return false;
    }

} // namespace EgLab::Common