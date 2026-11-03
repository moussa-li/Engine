#include "Command/CommandManager.hpp"
#include "Command/CommandQueueName.hpp"
#include "Common/Publisher.hpp"


namespace EgLab::Platform
{
    bool CommandManager::call(uint32_t processId, EventId eventId, const PacketPOD& packet)
    {
        Common::MemoryQueue queue(commandMemoryName(processId), false);
        Common::Publisher publisher(queue);
        return publisher.publish(static_cast<uint32_t>(eventId), packet);
    }
} // namespace EgLab::Platform