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
#include "Command/FloatList.hpp"
#include "Command/GetMeshDataCmd.hpp"
#include "Command/HighlightCmd.hpp"
#include "Command/ICommand.hpp"
#include "Command/IdList.hpp"
#include "Command/ImportMeshCmd.hpp"
#include "Command/ImportGeometryCmd.hpp"
#include "Command/GetGeometryInfoCmd.hpp"
#include "Command/ListGeometryFeaturesCmd.hpp"
#include "Command/SetGeometryRenderModeCmd.hpp"
#include "Command/SetGeometryPickFilterCmd.hpp"
#include "Command/SetGeometryVertexDisplayCmd.hpp"
#include "Command/LocateElementsCmd.hpp"
#include "Command/ElementsVisibilityCmd.hpp"
#include "Command/NodeVisibilityCmd.hpp"
#include "Command/MeshQueryCmd.hpp"
#include "Command/SetCameraCmd.hpp"
#include "Command/SetReferencePlaneCmd.hpp"
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

        // Wire layout for the variable length ElementsVisibility payload.
        struct ElementsVisibilityWireHeader
        {
            uint64_t dataNodeId;
            uint32_t mode;
            uint32_t count;
        };

        // Wire layout for the variable length NodeVisibility payload.
        struct NodeVisibilityWireHeader
        {
            uint64_t dataNodeId;
            uint32_t mode;
            uint32_t count;
        };

        // Wire layout for the variable length GetNodesXYZ / GetElementsTopology payload.
        // 80 字节对齐，后面紧跟 count 个 uint32 id。
        struct MeshIdsQueryWireHeader
        {
            uint64_t dataNodeId;
            char name[64];
            uint32_t count;
            uint32_t reserved;
        };
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

    class HidePanelCommand : public ICommand
    {
    public:
        explicit HidePanelCommand(const HidePanelParam& params) : _params(params)
        {
        }

        Common::Return exec() override
        {
            return Context::instance().getPanelManager().queueHidePanel(_params.panelName);
        }

    private:
        HidePanelParam _params;
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

            _creator[EventId::HidePanel] = [](const PacketPOD& pod) {
                HidePanelParam param{};
                std::memcpy(&param, pod.data, sizeof(param));
                return Common::makeShared<HidePanelCommand>(param);
            };

            _creator[EventId::SetCamera] = [](const PacketPOD& pod) {
                SetCameraParam param{};
                std::memcpy(&param, pod.data, sizeof(param));
                return Common::makeShared<SetCameraCmd>(param);
            };

            _creator[EventId::SetReferencePlane] = [](const PacketPOD& pod) {
                SetReferencePlaneParam param{};
                std::memcpy(&param, pod.data, sizeof(param));
                return Common::makeShared<SetReferencePlaneCmd>(param);
            };

            _creator[EventId::GetMeshData] = [](const PacketPOD& pod) {
                GetMeshDataParam param{};
                std::memcpy(&param, pod.data, sizeof(param));
                return Common::makeShared<GetMeshDataCmd>(param);
            };

            _creator[EventId::ListDataTreeNodes] = [](const PacketPOD&) {
                return Common::makeShared<ListDataTreeNodesCmd>();
            };

            _creator[EventId::GetMeshNodes] = [](const PacketPOD& pod) {
                MeshQueryParam param{};
                std::memcpy(&param, pod.data, sizeof(param));
                return Common::makeShared<GetMeshNodesCmd>(param);
            };

            _creator[EventId::GetMeshElements] = [](const PacketPOD& pod) {
                MeshQueryParam param{};
                std::memcpy(&param, pod.data, sizeof(param));
                return Common::makeShared<GetMeshElementsCmd>(param);
            };

            // GetNodesXYZ / GetElementsTopology 为变长 wire（见 worker 中的解析分支），
            // 不使用固定 POD 的 _creator。

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

            _creator[EventId::GeometryImport] = [](const PacketPOD& pod) {
                ImportGeometryParam param{};
                std::memcpy(&param, pod.data, sizeof(param));
                return Common::makeShared<ImportGeometryCmd>(param);
            };

            _creator[EventId::GetGeometryInfo] = [](const PacketPOD& pod) {
                GeometryQueryParam param{};
                std::memcpy(&param, pod.data, sizeof(param));
                return Common::makeShared<GetGeometryInfoCmd>(param);
            };

            _creator[EventId::ListGeometryFeatures] = [](const PacketPOD& pod) {
                GeometryFeaturesParam param{};
                std::memcpy(&param, pod.data, sizeof(param));
                return Common::makeShared<ListGeometryFeaturesCmd>(param);
            };

            _creator[EventId::SetGeometryRenderMode] = [](const PacketPOD& pod) {
                GeometryRenderModeParam param{};
                std::memcpy(&param, pod.data, sizeof(param));
                return Common::makeShared<SetGeometryRenderModeCmd>(param);
            };

            _creator[EventId::SetGeometryPickFilter] = [](const PacketPOD& pod) {
                GeometryPickFilterParam param{};
                std::memcpy(&param, pod.data, sizeof(param));
                return Common::makeShared<SetGeometryPickFilterCmd>(param);
            };

            _creator[EventId::SetGeometryVertexDisplay] = [](const PacketPOD& pod) {
                GeometryVertexDisplayParam param{};
                std::memcpy(&param, pod.data, sizeof(param));
                return Common::makeShared<SetGeometryVertexDisplayCmd>(param);
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

            _packetCreator[EventId::GeometryImport] = [](const EventPacket& packet) {
                PacketPOD pod{};
                ImportGeometryParam param{};
                if (packet.contains("filePath"))
                {
                    const Common::String filePath = packet.getData<Common::String>("filePath");
                    const size_t copySize = std::min(filePath.size(), sizeof(param.filePath) - 1);
                    std::memcpy(param.filePath, filePath.c_str(), copySize);
                    param.filePath[copySize] = '\0';
                }
                if (packet.contains("kernelName"))
                {
                    const Common::String kernelName =
                        packet.getData<Common::String>("kernelName");
                    const size_t copySize =
                        std::min(kernelName.size(), sizeof(param.kernelName) - 1);
                    std::memcpy(param.kernelName, kernelName.c_str(), copySize);
                    param.kernelName[copySize] = '\0';
                }
                pod.dataSize = sizeof(param);
                std::memcpy(pod.data, &param, sizeof(param));
                return pod;
            };

            _packetCreator[EventId::GetGeometryInfo] = [](const EventPacket& packet) {
                PacketPOD pod{};
                GeometryQueryParam param{};
                param.featureType = 0xFFFFFFFFu;
                param.featureId = 0xFFFFFFFFu;
                param.elemId = 0;
                if (packet.contains("dataNodeId"))
                {
                    param.dataNodeId = packet.getData<uint64_t>("dataNodeId");
                }
                if (packet.contains("name"))
                {
                    const Common::String name = packet.getData<Common::String>("name");
                    const size_t copySize = std::min(name.size(), sizeof(param.name) - 1);
                    std::memcpy(param.name, name.c_str(), copySize);
                    param.name[copySize] = '\0';
                }
                if (packet.contains("featureType"))
                {
                    param.featureType = packet.getData<uint32_t>("featureType");
                }
                if (packet.contains("featureId"))
                {
                    param.featureId = packet.getData<uint32_t>("featureId");
                }
                if (packet.contains("elemId"))
                {
                    param.elemId = packet.getData<uint32_t>("elemId");
                }
                pod.dataSize = sizeof(param);
                std::memcpy(pod.data, &param, sizeof(param));
                return pod;
            };

            _packetCreator[EventId::ListGeometryFeatures] = [](const EventPacket& packet) {
                PacketPOD pod{};
                GeometryFeaturesParam param{};
                param.featureType = 0xFFFFFFFFu;
                if (packet.contains("dataNodeId"))
                {
                    param.dataNodeId = packet.getData<uint64_t>("dataNodeId");
                }
                if (packet.contains("name"))
                {
                    const Common::String name = packet.getData<Common::String>("name");
                    const size_t copySize = std::min(name.size(), sizeof(param.name) - 1);
                    std::memcpy(param.name, name.c_str(), copySize);
                    param.name[copySize] = '\0';
                }
                if (packet.contains("featureType"))
                {
                    param.featureType = packet.getData<uint32_t>("featureType");
                }
                if (packet.contains("offset"))
                {
                    param.offset = packet.getData<uint32_t>("offset");
                }
                if (packet.contains("limit"))
                {
                    param.limit = packet.getData<uint32_t>("limit");
                }
                pod.dataSize = sizeof(param);
                std::memcpy(pod.data, &param, sizeof(param));
                return pod;
            };

            _packetCreator[EventId::SetGeometryRenderMode] = [](const EventPacket& packet) {
                PacketPOD pod{};
                GeometryRenderModeParam param{};
                if (packet.contains("mode"))
                {
                    param.mode = packet.getData<uint32_t>("mode");
                }
                pod.dataSize = sizeof(param);
                std::memcpy(pod.data, &param, sizeof(param));
                return pod;
            };

            _packetCreator[EventId::SetGeometryPickFilter] = [](const EventPacket& packet) {
                PacketPOD pod{};
                GeometryPickFilterParam param{};
                if (packet.contains("mask"))
                {
                    param.mask = packet.getData<uint32_t>("mask");
                }
                pod.dataSize = sizeof(param);
                std::memcpy(pod.data, &param, sizeof(param));
                return pod;
            };

            _packetCreator[EventId::SetGeometryVertexDisplay] = [](const EventPacket& packet) {
                PacketPOD pod{};
                GeometryVertexDisplayParam param{};
                if (packet.contains("visible"))
                {
                    param.visible = packet.getData<uint32_t>("visible");
                }
                pod.dataSize = sizeof(param);
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

            _packetCreator[EventId::HidePanel] = [](const EventPacket& packet) {
                PacketPOD pod{};
                HidePanelParam param{};
                const Common::String panelName = packet.getData<Common::String>("panelName");
                const size_t copySize = std::min(panelName.size(), sizeof(param.panelName) - 1);
                std::memcpy(param.panelName, panelName.c_str(), copySize);
                pod.dataSize = sizeof(param);
                std::memcpy(pod.data, &param, sizeof(param));
                return pod;
            };

            _packetCreator[EventId::SetCamera] = [](const EventPacket& packet) {
                PacketPOD pod{};
                SetCameraParam param{};
                param.mode = static_cast<uint32_t>(CameraMode::Orbit);
                if (packet.contains("mode"))
                {
                    const Common::String mode = packet.getData<Common::String>("mode");
                    if (mode == Common::String("fps"))
                    {
                        param.mode = static_cast<uint32_t>(CameraMode::Fps);
                    }
                }

                if (packet.contains("position"))
                {
                    float position[3]{};
                    parseFloat3(packet.getData<Common::String>("position"), position);
                    param.position[0] = position[0];
                    param.position[1] = position[1];
                    param.position[2] = position[2];
                    param.fieldMask |= CAMERA_FIELD_POSITION;
                }

                if (packet.contains("target"))
                {
                    float target[3]{};
                    parseFloat3(packet.getData<Common::String>("target"), target);
                    param.target[0] = target[0];
                    param.target[1] = target[1];
                    param.target[2] = target[2];
                    param.fieldMask |= CAMERA_FIELD_TARGET;
                }

                if (packet.contains("fov"))
                {
                    param.fov = packet.getData<float>("fov");
                    param.fieldMask |= CAMERA_FIELD_FOV;
                }

                param.duration = 0.45f;
                if (packet.contains("duration"))
                {
                    param.duration = packet.getData<float>("duration");
                    param.fieldMask |= CAMERA_FIELD_DURATION;
                }

                pod.dataSize = sizeof(param);
                std::memcpy(pod.data, &param, sizeof(param));
                return pod;
            };

            _packetCreator[EventId::GetMeshData] = [](const EventPacket& packet) {
                PacketPOD pod{};
                GetMeshDataParam param{packet.getData<uint64_t>("dataNodeId")};
                pod.dataSize = sizeof(param);
                std::memcpy(pod.data, &param, sizeof(param));
                return pod;
            };

            _packetCreator[EventId::SetReferencePlane] = [](const EventPacket& packet) {
                PacketPOD pod{};
                SetReferencePlaneParam param{};
                if (packet.contains("enabled"))
                {
                    param.enabled = packet.getData<bool>("enabled") ? 1u : 0u;
                    param.fieldMask |= REFERENCE_PLANE_FIELD_ENABLED;
                }
                if (packet.contains("showGrid"))
                {
                    param.showGrid = packet.getData<bool>("showGrid") ? 1u : 0u;
                    param.fieldMask |= REFERENCE_PLANE_FIELD_SHOW_GRID;
                }
                if (packet.contains("placement"))
                {
                    param.placement = packet.getData<uint32_t>("placement");
                    param.fieldMask |= REFERENCE_PLANE_FIELD_PLACEMENT;
                }
                if (packet.contains("scale"))
                {
                    param.scale = packet.getData<float>("scale");
                    param.fieldMask |= REFERENCE_PLANE_FIELD_SCALE;
                }
                if (packet.contains("colorX"))
                {
                    parseFloat4(packet.getData<Common::String>("colorX"), param.colors[0]);
                    param.fieldMask |= REFERENCE_PLANE_FIELD_COLOR_X;
                }
                if (packet.contains("colorY"))
                {
                    parseFloat4(packet.getData<Common::String>("colorY"), param.colors[1]);
                    param.fieldMask |= REFERENCE_PLANE_FIELD_COLOR_Y;
                }
                if (packet.contains("colorZ"))
                {
                    parseFloat4(packet.getData<Common::String>("colorZ"), param.colors[2]);
                    param.fieldMask |= REFERENCE_PLANE_FIELD_COLOR_Z;
                }
                pod.dataSize = sizeof(param);
                std::memcpy(pod.data, &param, sizeof(param));
                return pod;
            };

            _packetCreator[EventId::ListDataTreeNodes] = [](const EventPacket&) {
                return PacketPOD{};
            };

            const auto meshQueryPacketCreator = [](const EventPacket& packet) {
                PacketPOD pod{};
                MeshQueryParam param{};
                if (packet.contains("dataNodeId"))
                {
                    param.dataNodeId = packet.getData<uint64_t>("dataNodeId");
                }
                if (packet.contains("name"))
                {
                    const Common::String name = packet.getData<Common::String>("name");
                    const size_t copySize = std::min(name.size(), sizeof(param.name) - 1);
                    std::memcpy(param.name, name.c_str(), copySize);
                    param.name[copySize] = '\0';
                }
                pod.dataSize = sizeof(param);
                std::memcpy(pod.data, &param, sizeof(param));
                return pod;
            };
            _packetCreator[EventId::GetMeshNodes] = meshQueryPacketCreator;
            _packetCreator[EventId::GetMeshElements] = meshQueryPacketCreator;

            // GetNodesXYZ / GetElementsTopology 为变长 wire，在 call(EventId, EventPacket) 中
            // 单独构造，不走固定 POD 的 _packetCreator。

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
            _subscriber.subscrib(static_cast<uint32_t>(EventId::HidePanel));
            _subscriber.subscrib(static_cast<uint32_t>(EventId::FitView));
            _subscriber.subscrib(static_cast<uint32_t>(EventId::Highlight));
            _subscriber.subscrib(static_cast<uint32_t>(EventId::SetCamera));
            _subscriber.subscrib(static_cast<uint32_t>(EventId::SetReferencePlane));
            _subscriber.subscrib(static_cast<uint32_t>(EventId::GetMeshData));
            _subscriber.subscrib(static_cast<uint32_t>(EventId::ListDataTreeNodes));
            _subscriber.subscrib(static_cast<uint32_t>(EventId::GetMeshNodes));
            _subscriber.subscrib(static_cast<uint32_t>(EventId::GetMeshElements));
            _subscriber.subscrib(static_cast<uint32_t>(EventId::GetNodesXYZ));
            _subscriber.subscrib(static_cast<uint32_t>(EventId::GetElementsTopology));
            _subscriber.subscrib((uint32_t)EventId::MeshImport);
            _subscriber.subscrib((uint32_t)EventId::MeshDelete);
            _subscriber.subscrib((uint32_t)EventId::LocateElements);
            _subscriber.subscrib((uint32_t)EventId::ElementsVisibility);
            _subscriber.subscrib((uint32_t)EventId::NodeVisibility);
            _subscriber.subscrib((uint32_t)EventId::UpdateMesh);
            _subscriber.subscrib((uint32_t)EventId::GeometryImport);
            _subscriber.subscrib((uint32_t)EventId::GetGeometryInfo);
            _subscriber.subscrib((uint32_t)EventId::ListGeometryFeatures);
            _subscriber.subscrib((uint32_t)EventId::SetGeometryRenderMode);
            _subscriber.subscrib((uint32_t)EventId::SetGeometryPickFilter);
            _subscriber.subscrib((uint32_t)EventId::SetGeometryVertexDisplay);

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
                        else if (static_cast<EventId>(eventId) == EventId::ElementsVisibility)
                        {
                            if (dataSize < sizeof(ElementsVisibilityWireHeader)) continue;

                            ElementsVisibilityWireHeader wire{};
                            std::memcpy(&wire, rawPacket.data() + sizeof(dataSize), sizeof(wire));
                            const size_t idsByteSize =
                                static_cast<size_t>(wire.count) * sizeof(uint32_t);
                            if (idsByteSize != dataSize - sizeof(wire)) continue;

                            ElementsVisibilityParam param{};
                            param.dataNodeId = wire.dataNodeId;
                            param.mode = wire.mode;
                            param.count = wire.count;
                            param.elementIds = nullptr;
                            if (wire.count > 0)
                            {
                                param.elementIds = new uint32_t[wire.count];
                                std::memcpy(param.elementIds,
                                            rawPacket.data() + sizeof(dataSize) + sizeof(wire),
                                            idsByteSize);
                            }
                            command = Common::makeShared<ElementsVisibilityCmd>(param);
                        }
                        else if (static_cast<EventId>(eventId) == EventId::NodeVisibility)
                        {
                            if (dataSize < sizeof(NodeVisibilityWireHeader)) continue;

                            NodeVisibilityWireHeader wire{};
                            std::memcpy(&wire, rawPacket.data() + sizeof(dataSize), sizeof(wire));
                            const size_t idsByteSize =
                                static_cast<size_t>(wire.count) * sizeof(uint32_t);
                            if (idsByteSize != dataSize - sizeof(wire)) continue;

                            NodeVisibilityParam param{};
                            param.dataNodeId = wire.dataNodeId;
                            param.mode = wire.mode;
                            param.count = wire.count;
                            param.nodeIds = nullptr;
                            if (wire.count > 0)
                            {
                                param.nodeIds = new uint32_t[wire.count];
                                std::memcpy(param.nodeIds,
                                            rawPacket.data() + sizeof(dataSize) + sizeof(wire),
                                            idsByteSize);
                            }
                            command = Common::makeShared<NodeVisibilityCmd>(param);
                        }
                        else if (static_cast<EventId>(eventId) == EventId::GetNodesXYZ)
                        {
                            if (dataSize < sizeof(MeshIdsQueryWireHeader)) continue;

                            MeshIdsQueryWireHeader wire{};
                            std::memcpy(&wire, rawPacket.data() + sizeof(dataSize), sizeof(wire));
                            const size_t idsByteSize =
                                static_cast<size_t>(wire.count) * sizeof(uint32_t);
                            if (idsByteSize != dataSize - sizeof(wire)) continue;

                            MeshNodesQueryParam param{};
                            param.dataNodeId = wire.dataNodeId;
                            std::memcpy(param.name, wire.name, sizeof(param.name));
                            param.count = wire.count;
                            param.reserved = 0;
                            param.nodeIds = nullptr;
                            if (wire.count > 0)
                            {
                                param.nodeIds = new uint32_t[wire.count];
                                std::memcpy(param.nodeIds,
                                            rawPacket.data() + sizeof(dataSize) + sizeof(wire),
                                            idsByteSize);
                            }
                            command = Common::makeShared<GetNodesXYZCmd>(param);
                        }
                        else if (static_cast<EventId>(eventId) == EventId::GetElementsTopology)
                        {
                            if (dataSize < sizeof(MeshIdsQueryWireHeader)) continue;

                            MeshIdsQueryWireHeader wire{};
                            std::memcpy(&wire, rawPacket.data() + sizeof(dataSize), sizeof(wire));
                            const size_t idsByteSize =
                                static_cast<size_t>(wire.count) * sizeof(uint32_t);
                            if (idsByteSize != dataSize - sizeof(wire)) continue;

                            MeshElementsQueryParam param{};
                            param.dataNodeId = wire.dataNodeId;
                            std::memcpy(param.name, wire.name, sizeof(param.name));
                            param.count = wire.count;
                            param.reserved = 0;
                            param.elemIds = nullptr;
                            if (wire.count > 0)
                            {
                                param.elemIds = new uint32_t[wire.count];
                                std::memcpy(param.elemIds,
                                            rawPacket.data() + sizeof(dataSize) + sizeof(wire),
                                            idsByteSize);
                            }
                            command = Common::makeShared<GetElementsTopologyCmd>(param);
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

        if (eventId == EventId::ElementsVisibility)
        {
            ElementsVisibilityWireHeader wire{};
            wire.dataNodeId = packet.getData<uint64_t>("nodeId");
            wire.mode = packet.getData<uint32_t>("mode");
            const Common::String ids = packet.getData<Common::String>("ids");

            Common::DynamicArray<uint32_t> elementIds;
            parseIdList(ids, elementIds);
            wire.count = static_cast<uint32_t>(elementIds.size());

            const size_t idsByteSize = static_cast<size_t>(elementIds.size()) * sizeof(uint32_t);
            const uint32_t dataSize =
                static_cast<uint32_t>(sizeof(ElementsVisibilityWireHeader) + idsByteSize);

            Common::DynamicArray<uint8_t> wirePayload;
            wirePayload.resize(sizeof(dataSize) + dataSize);
            std::memcpy(wirePayload.data(), &dataSize, sizeof(dataSize));
            std::memcpy(wirePayload.data() + sizeof(dataSize), &wire, sizeof(wire));
            if (!elementIds.empty())
            {
                std::memcpy(wirePayload.data() + sizeof(dataSize) + sizeof(wire),
                            elementIds.data(), idsByteSize);
            }

            if (!_impl->_memQueue.push(static_cast<uint32_t>(eventId), wirePayload.data(),
                                       static_cast<uint32_t>(wirePayload.size())))
            {
                throw std::runtime_error("Failed to publish ElementsVisibility payload.");
            }
            return;
        }

        if (eventId == EventId::NodeVisibility)
        {
            NodeVisibilityWireHeader wire{};
            wire.dataNodeId = packet.getData<uint64_t>("nodeId");
            wire.mode = packet.getData<uint32_t>("mode");
            const Common::String ids = packet.getData<Common::String>("ids");

            Common::DynamicArray<uint32_t> nodeIds;
            parseIdList(ids, nodeIds);
            wire.count = static_cast<uint32_t>(nodeIds.size());

            const size_t idsByteSize = static_cast<size_t>(nodeIds.size()) * sizeof(uint32_t);
            const uint32_t dataSize =
                static_cast<uint32_t>(sizeof(NodeVisibilityWireHeader) + idsByteSize);

            Common::DynamicArray<uint8_t> wirePayload;
            wirePayload.resize(sizeof(dataSize) + dataSize);
            std::memcpy(wirePayload.data(), &dataSize, sizeof(dataSize));
            std::memcpy(wirePayload.data() + sizeof(dataSize), &wire, sizeof(wire));
            if (!nodeIds.empty())
            {
                std::memcpy(wirePayload.data() + sizeof(dataSize) + sizeof(wire), nodeIds.data(),
                            idsByteSize);
            }

            if (!_impl->_memQueue.push(static_cast<uint32_t>(eventId), wirePayload.data(),
                                       static_cast<uint32_t>(wirePayload.size())))
            {
                throw std::runtime_error("Failed to publish NodeVisibility payload.");
            }
            return;
        }

        if (eventId == EventId::GetNodesXYZ || eventId == EventId::GetElementsTopology)
        {
            MeshIdsQueryWireHeader wire{};
            if (packet.contains("dataNodeId"))
            {
                wire.dataNodeId = packet.getData<uint64_t>("dataNodeId");
            }
            if (packet.contains("name"))
            {
                const Common::String name = packet.getData<Common::String>("name");
                const size_t copySize = std::min(name.size(), sizeof(wire.name) - 1);
                std::memcpy(wire.name, name.c_str(), copySize);
                wire.name[copySize] = '\0';
            }

            Common::DynamicArray<uint32_t> parsedIds;
            if (packet.contains("ids"))
            {
                parseIdList(packet.getData<Common::String>("ids"), parsedIds);
            }
            wire.count = static_cast<uint32_t>(parsedIds.size());

            const size_t idsByteSize = static_cast<size_t>(parsedIds.size()) * sizeof(uint32_t);
            const uint32_t dataSize = static_cast<uint32_t>(sizeof(wire) + idsByteSize);

            Common::DynamicArray<uint8_t> wirePayload;
            wirePayload.resize(sizeof(dataSize) + dataSize);
            std::memcpy(wirePayload.data(), &dataSize, sizeof(dataSize));
            std::memcpy(wirePayload.data() + sizeof(dataSize), &wire, sizeof(wire));
            if (!parsedIds.empty())
            {
                std::memcpy(wirePayload.data() + sizeof(dataSize) + sizeof(wire), parsedIds.data(),
                            idsByteSize);
            }

            if (!_impl->_memQueue.push(static_cast<uint32_t>(eventId), wirePayload.data(),
                                       static_cast<uint32_t>(wirePayload.size())))
            {
                throw std::runtime_error("Failed to publish mesh query payload.");
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