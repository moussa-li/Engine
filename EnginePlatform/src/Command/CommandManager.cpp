#include "Command/CommandManager.hpp"

#include <algorithm>
#include <cstring>
#include <functional>
#include <thread>

#include "Command/CommandBus.hpp"
#include "Command/ICommand.hpp"
#include "Command/ImportMeshCmd.hpp"
#include "Command/UpdateMeshCmd.hpp"
#include "Common/HashMap.hpp"
#include "Common/Publisher.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Subscriber.hpp"

template <>
struct EgLab::Common::hash<EgLab::Platform::EventId>
{
    size_t operator()(const EgLab::Platform::EventId& eventId) const
    {
        using UnderlyingType = std::underlying_type_t<EgLab::Platform::EventId>;
        return std::hash<UnderlyingType>{}(static_cast<UnderlyingType>(eventId));
    }
};

namespace EgLab::Platform
{

    using ICommandCreateFunc = std::function<Common::SharedPtr<ICommand>(const PacketPOD& packet)>;
    using EventPacketToPODCreateFunc = std::function<PacketPOD(const EventPacket& packet)>;

    class CommandManager::Impl
    {
    public:
        Common::HashMap<EventId, ICommandCreateFunc> _creator;
        Common::HashMap<EventId, EventPacketToPODCreateFunc> _packetCreator;

        CommandBus _commandBus{2};
        CommandBus _renderCommandBus{1}; // render work command bus, for commands that need to be
                                         // executed in the render thread

        Common::MemoryQueue _memQueue;
        Common::Subscriber _subscriber;

        std::thread _worker;
        std::atomic<bool> _isRunning = false;

        Impl() : _memQueue("Command Memory", true), _subscriber(_memQueue)
        {
        }

        ~Impl()
        {
            if (_isRunning)
            {
                _isRunning = false; // 通知线程退出循环
                if (_worker.joinable())
                {
                    _worker.join(); // 等待线程真正执行完毕并回收资源
                }
            }
        }

        void registerCommand()
        {
            _creator[EventId::MeshImport] = [](const PacketPOD& pod) {
                ImportMeshParam param;
                std::memcpy(&param, pod.data, sizeof(ImportMeshParam));
                return Common::makeShared<ImportMeshCmd>(param);
            };

            _creator[EventId::UpdateMesh] = [](const PacketPOD& pod) {
                UpdateMeshParam param{};
                param.dataSize = pod.dataSize;
                param.data = new uint8_t[param.dataSize];
                std::memcpy(param.data, pod.data, param.dataSize);
                return Common::makeShared<UpdateMeshCmd>(param);
            };

            _packetCreator[EventId::MeshImport] = [](const EventPacket& packet) {
                PacketPOD pod{};
                ImportMeshParam param{};
                const Common::String fileDir = packet.getData<Common::String>("fileDir");
                std::memset(param.fileDir, 0, sizeof(param.fileDir));
                const size_t copySize = std::min(fileDir.size(), sizeof(param.fileDir) - 1);
                std::memcpy(param.fileDir, fileDir.c_str(), copySize);
                param.fileDir[copySize] = '\0';

                pod.dataSize = sizeof(ImportMeshParam);
                std::memcpy(pod.data, &param, sizeof(param));
                return pod;
            };

            _packetCreator[EventId::UpdateMesh] = [](const EventPacket& packet) {
                PacketPOD pod{};
                const auto& raw = packet.getRawBuffer();
                if (raw.size() > sizeof(pod.data))
                {
                    pod.dataSize = static_cast<uint32_t>(sizeof(pod.data));
                    std::memcpy(pod.data, raw.data(), sizeof(pod.data));
                }
                else
                {
                    pod.dataSize = static_cast<uint32_t>(raw.size());
                    std::memcpy(pod.data, raw.data(), raw.size());
                }
                return pod;
            };
        }

        void startListen()
        {
            _subscriber.subscrib((uint32_t)EventId::MeshImport);
            _subscriber.subscrib((uint32_t)EventId::UpdateMesh);

            if (_isRunning == true) return;
            _isRunning = true;

            _worker = std::thread([this]() {
                uint32_t eventId;
                EventPacket packet;
                PacketPOD pod;
                while (_isRunning)
                {
                    if (_subscriber.tryReceive(eventId, pod))
                    {
                        auto creator = _creator.find((EventId)eventId);
                        if (creator != _creator.end())
                        {
                            auto command = creator.data().second(pod);
                            _commandBus.dispatch(command);
                        }
                    }
                    else
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                }
            });
        }
    };

    void CommandManager::call(EventId eventId, PacketPOD packet)
    {
        Common::Publisher publisher(_impl->_memQueue);
        publisher.publish((uint32_t)eventId, packet);
    }

    void CommandManager::call(EventId eventId, EventPacket packet)
    {
        auto creator = _impl->_packetCreator.find(eventId);
        if (creator == _impl->_packetCreator.end())
        {
            return;
        }

        PacketPOD pod = creator.data().second(packet);
        this->call(eventId, pod);
    }

    CommandManager::CommandManager() : _impl(EgLab::Common::makeUnique<Impl>())
    {
        _impl->registerCommand();

        _impl->startListen();
    }

    CommandManager::~CommandManager()
    {
    }

} // namespace EgLab::Platform