#include "Command/CommandManager.hpp"

#include <algorithm>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <thread>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

#include "Command/CommandBus.hpp"
#include "Command/CommandQueueName.hpp"
#include "Command/DeleteMeshCmd.hpp"
#include "Command/HighlightCmd.hpp"
#include "Command/ICommand.hpp"
#include "Command/IdList.hpp"
#include "Command/ImportMeshCmd.hpp"
#include "Command/LocateElementsCmd.hpp"
#include "Command/UpdateMeshCmd.hpp"
#include "Common/HashMap.hpp"
#include "Common/Publisher.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Subscriber.hpp"
#include "DataBase/Context.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    namespace
    {
        uint32_t currentProcessId()
        {
#ifdef _WIN32
            return static_cast<uint32_t>(_getpid());
#else
            return static_cast<uint32_t>(getpid());
#endif
        }
    } // namespace

    class ShowPanelCommand : public ICommand
    {
    public:
        explicit ShowPanelCommand(const ShowPanelParam& params) : _params(params)
        {
        }

        Common::Return exec() override
        {
            return Context::instance().getPanelManager().queueShowPanel(_params.panelName);
        }

    private:
        ShowPanelParam _params;
    };

    class FitViewCommand : public ICommand
    {
    public:
        Common::Return exec() override
        {
            RenderWork::instance().fitView();
            return Common::Return::Succeed;
        }
    };

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

        Impl() : _memQueue(commandMemoryName(currentProcessId()), true), _subscriber(_memQueue)
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
            _creator[EventId::ShowPanel] = [](const PacketPOD& pod) {
                ShowPanelParam param{};
                std::memcpy(&param, pod.data, sizeof(param));
                return Common::makeShared<ShowPanelCommand>(param);
            };

            _creator[EventId::FitView] = [](const PacketPOD&) {
                return Common::makeShared<FitViewCommand>();
            };

            _creator[EventId::MeshDelete] = [](const PacketPOD& pod) {
                DeleteMeshParam param{};
                std::memcpy(&param, pod.data, sizeof(param));
                return Common::makeShared<DeleteMeshCmd>(param);
            };

            _creator[EventId::MeshImport] = [](const PacketPOD& pod) {
                ImportMeshParam param;
                std::memcpy(&param, pod.data, sizeof(ImportMeshParam));
                return Common::makeShared<ImportMeshCmd>(param);
            };

            _creator[EventId::Highlight] = [](const PacketPOD& pod) {
                HighlightParam param{};
                std::memcpy(&param, pod.data, sizeof(param));
                return Common::makeShared<HighlightCmd>(param);
            };

            _creator[EventId::LocateElements] = [](const PacketPOD& pod) {
                LocateElementsParam param{};
                std::memcpy(&param, pod.data, sizeof(param));
                return Common::makeShared<LocateElementsCmd>(param);
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

            _packetCreator[EventId::ShowPanel] = [](const EventPacket& packet) {
                PacketPOD pod{};
                ShowPanelParam param{};
                const Common::String panelName = packet.getData<Common::String>("panelName");
                const size_t copySize = std::min(panelName.size(), sizeof(param.panelName) - 1);
                std::memcpy(param.panelName, panelName.c_str(), copySize);
                pod.dataSize = sizeof(param);
                std::memcpy(pod.data, &param, sizeof(param));
                return pod;
            };

            _packetCreator[EventId::FitView] = [](const EventPacket&) { return PacketPOD{}; };

            _packetCreator[EventId::MeshDelete] = [](const EventPacket& packet) {
                PacketPOD pod{};
                DeleteMeshParam param{packet.getData<uint64_t>("dataNodeId")};
                pod.dataSize = sizeof(param);
                std::memcpy(pod.data, &param, sizeof(param));
                return pod;
            };

            _packetCreator[EventId::Highlight] = [](const EventPacket& packet) {
                PacketPOD pod{};
                HighlightParam param{};
                const Common::String ids = packet.getData<Common::String>("ids");
                param.count = static_cast<uint32_t>(
                    parseIdList(ids, param.dataNodeIds, MAX_HIGHLIGHT_DATA_NODE_COUNT));
                pod.dataSize = sizeof(param);
                std::memcpy(pod.data, &param, sizeof(param));
                return pod;
            };

            _packetCreator[EventId::LocateElements] = [](const EventPacket& packet) {
                PacketPOD pod{};
                LocateElementsParam param{};
                param.dataNodeId = packet.getData<uint64_t>("nodeId");
                const Common::String ids = packet.getData<Common::String>("ids");
                param.count = static_cast<uint32_t>(
                    parseIdList(ids, param.elementIds, MAX_LOCATE_ELEMENT_COUNT));
                pod.dataSize = sizeof(param);
                std::memcpy(pod.data, &param, sizeof(param));
                return pod;
            };
        }

        void startListen()
        {
            _subscriber.subscrib(static_cast<uint32_t>(EventId::ShowPanel));
            _subscriber.subscrib(static_cast<uint32_t>(EventId::FitView));
            _subscriber.subscrib(static_cast<uint32_t>(EventId::Highlight));
            _subscriber.subscrib((uint32_t)EventId::MeshImport);
            _subscriber.subscrib((uint32_t)EventId::MeshDelete);
            _subscriber.subscrib((uint32_t)EventId::LocateElements);
            _subscriber.subscrib((uint32_t)EventId::UpdateMesh);

            if (_isRunning == true) return;
            _isRunning = true;

            _worker = std::thread([this]() {
                uint32_t eventId;
                Common::DynamicArray<uint8_t> rawPacket;
                while (_isRunning)
                {
                    if (_subscriber.tryReceiveDynamic(eventId, rawPacket))
                    {
                        if (rawPacket.size() < sizeof(uint32_t)) continue;

                        uint32_t dataSize = 0;
                        std::memcpy(&dataSize, rawPacket.data(), sizeof(dataSize));
                        if (dataSize > rawPacket.size() - sizeof(dataSize)) continue;

                        Common::SharedPtr<ICommand> command;
                        if (static_cast<EventId>(eventId) == EventId::UpdateMesh)
                        {
                            if (dataSize == 0) continue;

                            UpdateMeshParam param{};
                            param.dataSize = dataSize;
                            param.data = new uint8_t[dataSize];
                            std::memcpy(param.data, rawPacket.data() + sizeof(dataSize), dataSize);
                            command = Common::makeShared<UpdateMeshCmd>(param);
                        }
                        else if (dataSize <= sizeof(PacketPOD::data))
                        {
                            PacketPOD pod{};
                            pod.dataSize = dataSize;
                            std::memcpy(pod.data, rawPacket.data() + sizeof(dataSize), dataSize);

                            auto creator = _creator.find(static_cast<EventId>(eventId));
                            if (creator != _creator.end())
                            {
                                command = creator.data().second(pod);
                            }
                        }
                        if (command) _commandBus.dispatch(command);
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
        if (eventId == EventId::UpdateMesh)
        {
            const auto& raw = packet.getRawBuffer();
            if (raw.size() > UINT32_MAX - sizeof(uint32_t))
            {
                throw std::length_error(
                    "UpdateMesh payload exceeds the shared-memory protocol limit.");
            }

            const uint32_t dataSize = static_cast<uint32_t>(raw.size());
            Common::DynamicArray<uint8_t> wirePayload;
            wirePayload.resize(sizeof(dataSize) + raw.size());
            std::memcpy(wirePayload.data(), &dataSize, sizeof(dataSize));
            if (!raw.empty())
            {
                std::memcpy(wirePayload.data() + sizeof(dataSize), raw.data(), raw.size());
            }
            if (!_impl->_memQueue.push(static_cast<uint32_t>(eventId), wirePayload.data(),
                                       static_cast<uint32_t>(wirePayload.size())))
            {
                throw std::runtime_error("Failed to publish UpdateMesh payload.");
            }
            return;
        }

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