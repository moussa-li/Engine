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

} // namespace EgLab::Common