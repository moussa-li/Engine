#include "Publisher.hpp"

namespace EgLab::Common
{
    Publisher::Publisher(MemoryQueue& queue) : _queue(queue)
    {
    }

} // namespace EgLab::Common