#include "Work/RenderWork.hpp"

#include <cstring>
#include <map>
#include <sstream>
#include <stdexcept>

#include "Command/MeshPODWire.hpp"
#include "Common/Log.hpp"
#include "Common/Performance.hpp"
#include "MeshEngine/Algorithm/ExtractSilhouette.hpp"
#include "MeshEngine/MeshData/HEMesh.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "MeshEngine/MeshData/MeshPOD.hpp"
#include "MeshEngine/MeshData/MeshToPOD.hpp"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/MeshPrimitiveCreator.hpp"
#include "RenderEngine/Core/MeshSelector.hpp"
#include "RenderEngine/Core/RenderFace.hpp"
#include "RenderEngine/Core/RenderLine.hpp"
#include "RenderEngine/Core/RenderNode.hpp"
#include "RenderEngine/Core/Renderer.hpp"
#include "RenderEngine/Core/Scene.hpp"
#include "RenderEngine/Core/ShaderLib.hpp"
#include "RenderEngine/Core/Window.hpp"
#include "Work/UIWork.hpp"

namespace EgLab::Platform
{
    Common::Return commitPendingRenderUpdates()
    {
        return RenderWork::instance().commitPendingRenderUpdates();
    }

    class RenderEventCommand : public ICommand
    {
    public:
        RenderEventCommand(RenderWork* worker, RenderWork::RenderEvent event)
            : _worker(worker), _event(Common::move(event))
        {
        }

        Common::Return exec() override
        {
            if (!_worker->beginRenderEventCommand())
            {
                return Common::Return::Failed;
            }

            try
            {
                _worker->processRenderEvent(Common::move(_event));
            }
            catch (const std::exception& exception)
            {
                LOG(ERROR) << "RenderEventCommand::exec() failed: " << exception.what();
                _worker->endRenderEventCommand();
                return Common::Return::Failed;
            }
            catch (...)
            {
                LOG(ERROR) << "RenderEventCommand::exec() failed with an unknown exception";
                _worker->endRenderEventCommand();
                return Common::Return::Failed;
            }
            _worker->endRenderEventCommand();
            return Common::Return::Succeed;
        }

    private:
        RenderWork* _worker;
        RenderWork::RenderEvent _event;
    };

    RenderWork::RenderWork()
    {
    }

    RenderWork::~RenderWork()
    {
        stop();
    }

    void RenderWork::bindWindow(const Common::SharedPtr<EgLab::RE::Window>& window)
    {
        _window = window;
    }

    void RenderWork::start()
    {
        if (_running.load())
        {
            return;
        }

        if (_window)
        {
            _window->start();
        }

        _stopping = false;
        _running = true;
        _renderEventCondition.notify_all();
        _thread = std::thread([this]() {
            UIWork::instance().start();

            while (_running.load())
            {
                // UI frame draw and ImGui state update run on the render
                // worker thread where the OpenGL context is owned.
                float currentFrame = _window->getTime();
                deltaTime = currentFrame - lastFrame;
                lastFrame = currentFrame;

                _window->activeContext();
                drainRenderEvents();
                if (!_running.load())
                {
                    _window->deactiveContext();
                    break;
                }
                // updateViewDependentHighlights();
                _scene->update(deltaTime);
                _renderer->update(deltaTime);
                if (!_running.load())
                {
                    _window->deactiveContext();
                    break;
                }
                _renderer->draw(_scene, _camera);
                _window->deactiveContext();

                UIWork::instance().renderFrame();

                if (!_running.load())
                {
                    break;
                }
                if (_window)
                {
                    _window->swapBuffers();
                }
            }
        });
    }

    void RenderWork::bindRenderer(const Common::SharedPtr<EgLab::RE::Renderer>& renderer)
    {
        _renderer = renderer;
        if (_renderer)
        {
            _meshSelector = Common::makeShared<RE::MeshSelector>();
            _renderer->setSelector(Common::SharedPtr<RE::Selector>(_meshSelector));
            _meshSelector->registerPickInfoCallback(
                [this](const RE::PrimitivePickInfo& info) { publishPickInfo(info); });
        }
        else
        {
            _meshSelector = nullptr;
        }
    }

    void RenderWork::bindScene(const Common::SharedPtr<EgLab::RE::Scene>& scene)
    {
        _scene = scene;
    }

    void RenderWork::bindCamera(const Common::SharedPtr<EgLab::RE::Camera>& camera)
    {
        _camera = camera;
    }

    void RenderWork::stop()
    {
        _stopping = true;
        _running = false;
        _renderEventCondition.notify_all();
        if (_thread.joinable())
        {
            _thread.join();
        }
        _renderEventBus.stop(true);
    }

    void RenderWork::releaseResources()
    {
        stop();

        if (_renderer)
        {
            _renderer->setSelector(nullptr);
        }
        _meshSelector = nullptr;
        _activeMeshPackets.clear();
        _meshColors.clear();
        _removedMeshes.clear();
        _requestedHighlightedMeshes.clear();
        _activeHighlightedMeshes.clear();
        _highlightPrimitives.clear();
        _highlightContourPrimitives.clear();
        _heMeshHighlightPrimitives.clear();
        _highlightShader = nullptr;
        _heMeshHighlightShader = nullptr;

        {
            std::lock_guard<std::mutex> lock(_renderEventMutex);
            _renderEventQueue.clear();
            _renderEventsPending.store(false, std::memory_order_release);
        }
        _renderEventCondition.notify_all();

        _scene = nullptr;
        _renderer = nullptr;
        _camera = nullptr;
        _window = nullptr;
    }

    Common::Return RenderWork::commitPendingRenderUpdates()
    {
        _renderCommitRequests.fetch_add(1, std::memory_order_acq_rel);
        std::unique_lock<std::mutex> lock(_renderEventMutex);
        _renderEventCondition.wait(lock, [this] { return _activeRenderEventCommands == 0; });
        if (!_renderEventQueue.empty())
        {
            _renderEventsPending.store(true, std::memory_order_release);
        }
        _renderCommitRequests.fetch_sub(1, std::memory_order_acq_rel);
        lock.unlock();
        _renderEventCondition.notify_all();
        return Common::Return::Succeed;
    }

    void RenderWork::subscribe(EventId eventId)
    {
        (void)eventId;
    }

    PickInfoSubscriptionId RenderWork::subscribePickInfo()
    {
        std::lock_guard<std::mutex> lock(_pickInfoSubscriptionsMutex);
        const PickInfoSubscriptionId subscriptionId = _nextPickInfoSubscriptionId++;

        std::ostringstream queueName;
        queueName << "RenderPickInfo_" << reinterpret_cast<uintptr_t>(this) << '_'
                  << subscriptionId;

        auto subscription = std::make_unique<PickInfoSubscription>();
        subscription->id = subscriptionId;
        subscription->queue =
            std::make_unique<Common::MemoryQueue>(Common::String(queueName.str().c_str()), true);
        subscription->publisher = std::make_unique<Common::Publisher>(*subscription->queue);
        subscription->subscriber = std::make_unique<Common::Subscriber>(*subscription->queue);
        subscription->subscriber->subscrib(static_cast<uint32_t>(EventId::MeshPickInfo));

        if (_hasPublishedPickInfo)
        {
            subscription->publisher->publish(static_cast<uint32_t>(EventId::MeshPickInfo),
                                             _lastPublishedPickInfo);
        }
        _pickInfoSubscriptions.pushBack(Common::move(subscription));
        return subscriptionId;
    }

    void RenderWork::unsubscribePickInfo(PickInfoSubscriptionId subscriptionId)
    {
        std::lock_guard<std::mutex> lock(_pickInfoSubscriptionsMutex);
        Common::DynamicArray<std::unique_ptr<PickInfoSubscription>> remaining;
        for (auto& subscription : _pickInfoSubscriptions)
        {
            if (subscription->id != subscriptionId)
            {
                remaining.pushBack(Common::move(subscription));
            }
        }
        _pickInfoSubscriptions.clear();
        _pickInfoSubscriptions = Common::move(remaining);
    }

    bool RenderWork::tryReceivePickInfo(PickInfoSubscriptionId subscriptionId, PickedMeshInfo& info)
    {
        std::lock_guard<std::mutex> lock(_pickInfoSubscriptionsMutex);
        for (const auto& subscription : _pickInfoSubscriptions)
        {
            if (subscription->id == subscriptionId)
            {
                uint32_t eventId = 0;
                return subscription->subscriber->tryReceive(eventId, info);
            }
        }
        return false;
    }

    void RenderWork::publishPickInfo(const RE::PrimitivePickInfo& info)
    {
        PickedMeshInfo pickedInfo;
        pickedInfo.entityId = info.entityId;
        pickedInfo.faceIndex = info.faceIdx;
        pickedInfo.edgeIndex = info.edgeIdx;
        pickedInfo.x = info.position.x();
        pickedInfo.y = info.position.y();
        pickedInfo.z = info.position.z();
        pickedInfo.topologyNodeCount = static_cast<uint32_t>(info.elementNodeCount);
        for (uint32_t index = 0; index < pickedInfo.topologyNodeCount; ++index)
        {
            pickedInfo.topologyNodeIds[index] = info.elementNodeIds[index];
        }
        switch (info.kind)
        {
        case RE::PrimitivePickKind::Node:
            pickedInfo.kind = PickedMeshKind::Node;
            break;
        case RE::PrimitivePickKind::Edge:
            pickedInfo.kind = PickedMeshKind::ElementEdge;
            break;
        case RE::PrimitivePickKind::Face:
            pickedInfo.kind = PickedMeshKind::ElementFace;
            break;
        default:
            pickedInfo.kind = PickedMeshKind::None;
            break;
        }

        std::lock_guard<std::mutex> lock(_pickInfoSubscriptionsMutex);
        _lastPublishedPickInfo = pickedInfo;
        _hasPublishedPickInfo = true;
        for (const auto& subscription : _pickInfoSubscriptions)
        {
            subscription->publisher->publish(static_cast<uint32_t>(EventId::MeshPickInfo),
                                             pickedInfo);
        }
    }

    void RenderWork::queueRenderEvent(RenderEvent event)
    {
        auto command = Common::SharedPtr<ICommand>(
            Common::makeShared<RenderEventCommand>(this, Common::move(event)));
        if (_renderEventBus.dispatch(command) != Common::Return::Succeed)
        {
            LOG(WARNING)
                << "RenderWork::queueRenderEvent() dropped event because its queue is stopped";
        }
    }

    void RenderWork::drainRenderEvents()
    {
        std::deque<RenderEvent> events;
        {
            std::lock_guard<std::mutex> lock(_renderEventMutex);
            if (!_renderEventsPending.load(std::memory_order_acquire))
            {
                return;
            }
            events.swap(_renderEventQueue);
            _renderEventsPending.store(false, std::memory_order_release);
        }
        _renderEventCondition.notify_all();

        for (const auto& event : events)
        {
            if (!_running.load())
            {
                break;
            }
            applyRenderUpdate(event);
        }
    }

    bool RenderWork::beginRenderEventCommand()
    {
        std::unique_lock<std::mutex> lock(_renderEventMutex);
        _renderEventCondition.wait(lock, [this] {
            return _stopping.load() ||
                   (_running.load() && _renderCommitRequests.load(std::memory_order_acquire) == 0);
        });
        if (_stopping.load())
        {
            return false;
        }

        ++_activeRenderEventCommands;
        _renderEventCondition.wait(lock, [this] {
            return _stopping.load() || !_renderEventsPending.load(std::memory_order_acquire);
        });
        if (_stopping.load())
        {
            --_activeRenderEventCommands;
            lock.unlock();
            _renderEventCondition.notify_all();
            return false;
        }
        return true;
    }

    void RenderWork::endRenderEventCommand()
    {
        {
            std::lock_guard<std::mutex> lock(_renderEventMutex);
            --_activeRenderEventCommands;
        }
        _renderEventCondition.notify_all();
    }

    void RenderWork::processRenderEvent(RenderEvent event)
    {
        if (event.type == RenderEventType::MeshUpdate)
        {
            Common::SharedPtr<ME::Mesh> renderMesh = event.mesh;
            if (!event.payload.empty())
            {
                if (event.payload.size() <= sizeof(MeshPODWireHeader))
                {
                    LOG(ERROR) << "RenderWork::processRenderEvent() received truncated mesh data";
                    return;
                }

                MeshPODWireHeader wire{};
                std::memcpy(&wire, event.payload.data(), sizeof(wire));
                if (wire.totalDataSize == 0 ||
                    wire.totalDataSize > event.payload.size() - sizeof(MeshPODWireHeader))
                {
                    LOG(ERROR)
                        << "RenderWork::processRenderEvent() received invalid mesh data size";
                    return;
                }

                const uint32_t offsets[] = {
                    wire.nodeIdsOffset,   wire.nodeDataOffset, wire.elemIdsOffset,
                    wire.elemTypesOffset, wire.elemDataOffset,
                };
                for (uint32_t offset : offsets)
                {
                    if (offset > wire.totalDataSize)
                    {
                        LOG(ERROR)
                            << "RenderWork::processRenderEvent() received invalid mesh offset";
                        return;
                    }
                }

                ME::MeshPOD pod{};
                pod.nodeNumber = wire.nodeNumber;
                pod.elemNumber = wire.elemNumber;
                pod.nodeIdsOffset = wire.nodeIdsOffset;
                pod.nodeDataOffset = wire.nodeDataOffset;
                pod.elemIdsOffset = wire.elemIdsOffset;
                pod.elemTypesOffset = wire.elemTypesOffset;
                pod.elemDataOffset = wire.elemDataOffset;
                pod.totalDataSize = wire.totalDataSize;
                pod.data = new uint8_t[pod.totalDataSize];
                std::memcpy(pod.data, event.payload.data() + sizeof(MeshPODWireHeader),
                            pod.totalDataSize);

                renderMesh = ME::MeshToPOD::deserialize(pod);
                if (!renderMesh)
                {
                    LOG(ERROR) << "RenderWork::processRenderEvent() failed to deserialize mesh";
                    return;
                }
            }
            if (!renderMesh)
            {
                LOG(ERROR) << "RenderWork::processRenderEvent() cannot update a null mesh";
                return;
            }

            RE::MeshPrimitiveCreator creator(renderMesh);
            event.packet = {creator.getPrimitive<RE::RenderNode>(),
                            creator.getPrimitive<RE::RenderLine>(),
                            creator.getPrimitive<RE::RenderFace>(),
                            event.mesh,
                            renderMesh,
                            creator.getTranslator()};
        }

        {
            std::lock_guard<std::mutex> stateLock(_renderStateMutex);
            switch (event.type)
            {
            case RenderEventType::MeshColor:
                if (!event.mesh || isMeshRemoved(event.mesh.get()))
                {
                    return;
                }
                {
                    bool found = false;
                    for (auto& meshColor : _meshColors)
                    {
                        if (meshColor.mesh == event.mesh)
                        {
                            meshColor.color = event.color;
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        _meshColors.pushBack(MeshColor{event.mesh, event.color});
                    }
                }
                break;
            case RenderEventType::MeshRemoval:
                if (!event.mesh || isMeshRemoved(event.mesh.get()))
                {
                    return;
                }
                _removedMeshes.pushBack(event.mesh.get());
                {
                    Common::DynamicArray<MeshColor> remainingColors;
                    for (const auto& meshColor : _meshColors)
                    {
                        if (meshColor.mesh != event.mesh)
                        {
                            remainingColors.pushBack(meshColor);
                        }
                    }
                    _meshColors = Common::move(remainingColors);

                    Common::DynamicArray<Common::SharedPtr<ME::Mesh>> remainingMeshes;
                    for (const auto& mesh : _requestedHighlightedMeshes)
                    {
                        if (mesh != event.mesh)
                        {
                            remainingMeshes.pushBack(mesh);
                        }
                    }
                    _requestedHighlightedMeshes = Common::move(remainingMeshes);
                }
                break;
            case RenderEventType::Highlight:
                _requestedHighlightedMeshes.clear();
                for (const auto& mesh : event.meshes)
                {
                    if (mesh && !isMeshRemoved(mesh.get()))
                    {
                        _requestedHighlightedMeshes.pushBack(mesh);
                    }
                }
                break;
            case RenderEventType::HighlightColor:
                _highlightColor = event.color;
                break;
            case RenderEventType::MeshUpdate:
            case RenderEventType::FitView:
            case RenderEventType::HEMeshHighlight:
                break;
            }
        }

        {
            std::lock_guard<std::mutex> lock(_renderEventMutex);
            _renderEventQueue.push_back(Common::move(event));
            _renderEventsPending.store(true, std::memory_order_release);
        }
    }

    Common::Return RenderWork::applyRenderUpdate(const RenderEvent& event)
    {
        std::lock_guard<std::mutex> stateLock(_renderStateMutex);
        switch (event.type)
        {
        case RenderEventType::FitView:
            if (_camera && _scene)
            {
                _camera->fitView(_scene->getBounds(), 2);
            }
            break;
        case RenderEventType::MeshUpdate:
            return uploadRenderPacket(event.packet);
        case RenderEventType::MeshColor:
        {
            if (!event.mesh)
            {
                break;
            }
            for (auto& packet : _activeMeshPackets)
            {
                if (packet.sourceMesh != event.mesh)
                {
                    continue;
                }
                auto face = Common::dynamicSharedPtrCast<RE::RenderFace>(packet.renderFace);
                if (face)
                {
                    face->setColor(event.color);
                }
            }
            break;
        }
        case RenderEventType::MeshRemoval:
        {
            if (!event.mesh)
            {
                break;
            }

            Common::SharedPtr<RE::Shader> nodeShader;
            Common::SharedPtr<RE::Shader> lineShader;
            Common::SharedPtr<RE::Shader> faceShader;
            RE::ShaderLib::instance().getNodeShader(nodeShader);
            RE::ShaderLib::instance().getLineShader(lineShader);
            RE::ShaderLib::instance().getFaceShader(faceShader);
            Common::DynamicArray<RenderPacket> remainingPackets;
            for (auto packet : _activeMeshPackets)
            {
                if (packet.sourceMesh != event.mesh)
                {
                    remainingPackets.pushBack(packet);
                    continue;
                }
                if (_meshSelector && packet.mesh)
                {
                    _meshSelector->unregisterMesh(packet.mesh.get());
                }
                _scene->removePrimitive(nodeShader, packet.renderNode);
                _scene->removePrimitive(lineShader, packet.renderLine);
                _scene->removePrimitive(faceShader, packet.renderFace);
            }
            _activeMeshPackets = Common::move(remainingPackets);
            updateMeshHighlight();
            break;
        }
        case RenderEventType::Highlight:
            updateMeshHighlight();
            break;
        case RenderEventType::HighlightColor:
            for (const auto& primitive : _highlightPrimitives)
            {
                auto face = Common::dynamicSharedPtrCast<RE::RenderFace>(primitive);
                if (face)
                {
                    face->setColor(_highlightColor);
                }
            }
            break;
        case RenderEventType::HEMeshHighlight:
            updateHEMeshHighlight(event);
            break;
        }
        return Common::Return::Succeed;
    }

    Common::Return RenderWork::uploadRenderPacket(const RenderPacket& packet)
    {
        if (!_running.load() || !_scene || !_camera)
        {
            return Common::Return::Failed;
        }
        if (packet.sourceMesh && isMeshRemoved(packet.sourceMesh.get()))
        {
            return Common::Return::Succeed;
        }

        if (packet.sourceMesh)
        {
            Common::Vector4f color(0.8f, 0.8f, 0.8f, 1.0f);
            for (const auto& meshColor : _meshColors)
            {
                if (meshColor.mesh == packet.sourceMesh)
                {
                    color = meshColor.color;
                    break;
                }
            }
            auto face = Common::dynamicSharedPtrCast<RE::RenderFace>(packet.renderFace);
            if (face)
            {
                face->setColor(color);
            }
        }

        packet.renderNode->setup();
        packet.renderLine->setup();
        packet.renderFace->setup();

        if (_meshSelector && packet.mesh && packet.translator)
        {
            _meshSelector->registerMesh(packet.mesh, packet.translator, packet.renderNode,
                                        packet.renderLine, packet.renderFace);
        }

        Common::SharedPtr<RE::Shader> nodeShader;
        Common::SharedPtr<RE::Shader> lineShader;
        Common::SharedPtr<RE::Shader> faceShader;
        RE::ShaderLib::instance().getNodeShader(nodeShader);
        RE::ShaderLib::instance().getLineShader(lineShader);
        RE::ShaderLib::instance().getFaceShader(faceShader);
        _scene->addPrimitive(nodeShader, packet.renderNode);
        _scene->addPrimitive(lineShader, packet.renderLine);
        _scene->addPrimitive(faceShader, packet.renderFace);
        if (packet.sourceMesh)
        {
            _activeMeshPackets.pushBack(packet);
        }
        //_camera->fitView(_scene->getBounds(), 2);
        return Common::Return::Succeed;
    }

    void RenderWork::fitView()
    {
        RenderEvent event{};
        event.type = RenderEventType::FitView;
        queueRenderEvent(Common::move(event));
    }

    void RenderWork::onUpdateMesh(const UpdateMeshParam& params)
    {
        if (params.data == nullptr || params.dataSize == 0)
        {
            return;
        }

        RenderEvent event{};
        event.type = RenderEventType::MeshUpdate;
        event.payload.pushBack(params.data, params.dataSize);
        queueRenderEvent(Common::move(event));
    }

    void RenderWork::onUpdateMesh(const Common::SharedPtr<EgLab::ME::Mesh>& mesh)
    {
        if (!mesh)
        {
            return;
        }

        RenderEvent event{};
        event.type = RenderEventType::MeshUpdate;
        event.mesh = mesh;
        queueRenderEvent(Common::move(event));
    }

    void RenderWork::setHighlightedMesh(const Common::SharedPtr<EgLab::ME::Mesh>& mesh)
    {
        Common::DynamicArray<Common::SharedPtr<ME::Mesh>> meshes;
        if (mesh)
        {
            auto selectedMesh = mesh;
            meshes.pushBack(selectedMesh);
        }
        setHighlightedMeshes(meshes);
    }

    void RenderWork::setHighlightedMeshes(
        const Common::DynamicArray<Common::SharedPtr<EgLab::ME::Mesh>>& meshes)
    {
        RenderEvent event{};
        event.type = RenderEventType::Highlight;
        event.meshes = meshes;
        queueRenderEvent(Common::move(event));
    }

    void RenderWork::setHighlightColor(const Common::Vector4f& color)
    {
        RenderEvent event{};
        event.type = RenderEventType::HighlightColor;
        event.color = color;
        queueRenderEvent(Common::move(event));
    }

    void RenderWork::setHEMeshHighlight(const Common::SharedPtr<ME::HEMesh>& mesh,
                                        uint32_t vertexIndex, uint32_t halfEdgeIndex)
    {
        RenderEvent event{};
        event.type = RenderEventType::HEMeshHighlight;
        event.heMesh = mesh;
        event.heMeshVertexIndex = vertexIndex;
        event.heMeshHalfEdgeIndex = halfEdgeIndex;
        queueRenderEvent(Common::move(event));
    }

    void RenderWork::setMeshColor(const Common::SharedPtr<EgLab::ME::Mesh>& mesh,
                                  const Common::Vector4f& color)
    {
        if (!mesh)
        {
            LOG(WARNING) << "RenderWork::setMeshColor() skipped null mesh";
            return;
        }

        RenderEvent event{};
        event.type = RenderEventType::MeshColor;
        event.mesh = mesh;
        event.color = color;
        queueRenderEvent(Common::move(event));
    }

    void RenderWork::removeMesh(const Common::SharedPtr<ME::Mesh>& mesh)
    {
        if (!mesh)
        {
            return;
        }

        RenderEvent event{};
        event.type = RenderEventType::MeshRemoval;
        event.mesh = mesh;
        queueRenderEvent(Common::move(event));
    }

    bool RenderWork::isMeshRemoved(const ME::Mesh* mesh)
    {
        for (ME::Mesh* removedMesh : _removedMeshes)
        {
            if (removedMesh == mesh)
            {
                return true;
            }
        }
        return false;
    }

    void RenderWork::updateMeshHighlight()
    {
        bool highlightsMatch =
            _requestedHighlightedMeshes.size() == _activeHighlightedMeshes.size();
        for (size_t index = 0; highlightsMatch && index < _requestedHighlightedMeshes.size();
             ++index)
        {
            highlightsMatch = _requestedHighlightedMeshes[index] == _activeHighlightedMeshes[index];
        }
        if (highlightsMatch)
        {
            return;
        }

        for (size_t index = 0; index < _highlightPrimitives.size(); ++index)
        {
            auto primitive = _highlightPrimitives[index];
            const Common::Return result = _scene->removePrimitive(_highlightShader, primitive);
            if (result != Common::Return::Succeed)
            {
                LOG(WARNING) << "RenderWork::updateMeshHighlight() could not remove old highlight";
            }
        }
        _highlightPrimitives.clear();
        for (size_t index = 0; index < _highlightContourPrimitives.size(); ++index)
        {
            _scene->removePrimitive(_highlightShader, _highlightContourPrimitives[index]);
        }
        _highlightContourPrimitives.clear();
        _highlightViewValid = false;
        _activeHighlightedMeshes = _requestedHighlightedMeshes;

        if (_activeHighlightedMeshes.empty())
        {
            return;
        }

        if (!_highlightShader)
        {
            RE::ShaderLib::instance().getHighlightShader(_highlightShader);
        }

        for (size_t index = 0; index < _activeHighlightedMeshes.size(); ++index)
        {
            auto mesh = _activeHighlightedMeshes[index];
            RE::MeshPrimitiveCreator creator(mesh);
            auto primitive = creator.getPrimitive<RE::RenderFace>();
            auto face = Common::dynamicSharedPtrCast<RE::RenderFace>(primitive);
            face->setColor(_highlightColor);
            face->setDepthOffset(-1.0f, -1.0f);
            primitive->setup();

            if (_scene->addPrimitive(_highlightShader, primitive) != Common::Return::Succeed)
            {
                LOG(ERROR) << "RenderWork::updateMeshHighlight() failed to add mesh highlight";
                continue;
            }
            _highlightPrimitives.pushBack(primitive);

            auto linePrimitive = Common::dynamicSharedPtrCast<RE::RenderLine>(
                creator.getPrimitive<RE::RenderLine>());
            linePrimitive->setIndices(creator.getTranslator()->getBoundaryElementEdgeIdx());
            linePrimitive->setColor(Common::Vector4f(1.0f, 0.75f, 0.1f, 1.0f));
            linePrimitive->setLineWidth(3.0f);
            linePrimitive->setup();
            if (_scene->addPrimitive(_highlightShader, linePrimitive) == Common::Return::Succeed)
            {
                _highlightContourPrimitives.pushBack(linePrimitive);
            }
        }
    }

    void RenderWork::updateViewDependentHighlights()
    {
        if (!_scene || !_camera || _activeHighlightedMeshes.empty())
        {
            return;
        }

        const RE::CoordType cameraPosition = _camera->getPosition();
        const RE::CoordType cameraFront = _camera->getFront();
        if (_highlightViewValid && (cameraPosition - _highlightViewPosition).length2() < 1e-8f &&
            (cameraFront - _highlightViewFront).length2() < 1e-8f)
        {
            return;
        }

        for (const auto& primitive : _highlightContourPrimitives)
        {
            _scene->removePrimitive(_highlightShader, primitive);
        }
        _highlightContourPrimitives.clear();
        _highlightViewPosition = cameraPosition;
        _highlightViewFront = cameraFront;
        _highlightViewValid = true;

        for (const auto& mesh : _activeHighlightedMeshes)
        {
            ME::ExtractSilhouette extractor(mesh);
            const auto edges = extractor.getSilhouetteEdges(
                ME::CoordType(cameraPosition.x(), cameraPosition.y(), cameraPosition.z()));
            if (edges.empty())
            {
                continue;
            }

            Common::DynamicArray<RE::CoordType> nodeCoordinates;
            std::map<ME::IdType, RE::IdxType> nodeIndices;
            ME::MeshIterator meshIterator(*mesh);
            for (; meshIterator.hasMoreNode(); meshIterator.nextNode())
            {
                auto& node = meshIterator.currentNode();
                nodeIndices[node.getId()] = static_cast<RE::IdxType>(nodeCoordinates.size());
                nodeCoordinates.pushBack(RE::CoordType(node.x(), node.y(), node.z()));
            }

            Common::DynamicArray<RE::IdxType> indices;
            for (const auto& edge : edges)
            {
                const auto firstIndex = nodeIndices.find(edge.first);
                const auto secondIndex = nodeIndices.find(edge.second);
                if (firstIndex == nodeIndices.end() || secondIndex == nodeIndices.end())
                {
                    continue;
                }
                indices.pushBack(firstIndex->second);
                indices.pushBack(secondIndex->second);
            }
            if (indices.empty())
            {
                continue;
            }

            auto linePrimitive = Common::makeShared<RE::RenderLine>();
            linePrimitive->setNodes(Common::move(nodeCoordinates));
            linePrimitive->setIndices(Common::move(indices));
            linePrimitive->setColor(Common::Vector4f(1.0f, 0.75f, 0.1f, 1.0f));
            linePrimitive->setup();
            if (_scene->addPrimitive(_highlightShader, linePrimitive) == Common::Return::Succeed)
            {
                _highlightContourPrimitives.pushBack(linePrimitive);
            }
        }
    }

    void RenderWork::updateHEMeshHighlight(const RenderEvent& event)
    {
        if (!_scene)
        {
            return;
        }
        for (const auto& primitive : _heMeshHighlightPrimitives)
        {
            _scene->removePrimitive(_heMeshHighlightShader, primitive);
        }
        _heMeshHighlightPrimitives.clear();

        if (!event.heMesh || event.heMeshVertexIndex >= event.heMesh->getVertexCount())
        {
            return;
        }
        if (!_heMeshHighlightShader)
        {
            RE::ShaderLib::instance().getHighlightShader(_heMeshHighlightShader);
        }

        const Common::Vector4f highlightColor(1.0f, 0.28f, 0.04f, 1.0f);
        const auto selectedVertex = event.heMesh->getVertex(event.heMeshVertexIndex);
        Common::DynamicArray<RE::CoordType> nodeCoordinates;
        RE::CoordType nodeCoordinate(selectedVertex._coord.x(), selectedVertex._coord.y(),
                                     selectedVertex._coord.z());
        nodeCoordinates.pushBack(nodeCoordinate);
        auto nodePrimitive = Common::makeShared<RE::RenderNode>();
        nodePrimitive->setNodes(Common::move(nodeCoordinates));
        nodePrimitive->setColor(highlightColor);
        nodePrimitive->setPointSize(12.0f);
        nodePrimitive->setDepthTestEnabled(false);
        nodePrimitive->setup();
        if (_scene->addPrimitive(_heMeshHighlightShader, nodePrimitive) == Common::Return::Succeed)
        {
            _heMeshHighlightPrimitives.pushBack(nodePrimitive);
        }

        if (event.heMeshHalfEdgeIndex >= event.heMesh->getHalfEdgeCount())
        {
            return;
        }
        const auto halfEdge = event.heMesh->getHalfEdge(event.heMeshHalfEdgeIndex);
        if (halfEdge.vertex != event.heMeshVertexIndex ||
            halfEdge.next >= event.heMesh->getHalfEdgeCount())
        {
            return;
        }
        const auto nextHalfEdge = event.heMesh->getHalfEdge(halfEdge.next);
        const auto nextVertex = event.heMesh->getVertex(nextHalfEdge.vertex);
        Common::DynamicArray<RE::CoordType> edgeCoordinates;
        RE::CoordType edgeStart(selectedVertex._coord.x(), selectedVertex._coord.y(),
                                selectedVertex._coord.z());
        RE::CoordType edgeEnd(nextVertex._coord.x(), nextVertex._coord.y(), nextVertex._coord.z());
        edgeCoordinates.pushBack(edgeStart);
        edgeCoordinates.pushBack(edgeEnd);
        Common::DynamicArray<RE::IdxType> edgeIndices;
        RE::IdxType firstIndex = 0;
        RE::IdxType secondIndex = 1;
        edgeIndices.pushBack(firstIndex);
        edgeIndices.pushBack(secondIndex);
        auto linePrimitive = Common::makeShared<RE::RenderLine>();
        linePrimitive->setNodes(Common::move(edgeCoordinates));
        linePrimitive->setIndices(Common::move(edgeIndices));
        linePrimitive->setColor(highlightColor);
        linePrimitive->setLineWidth(5.0f);
        linePrimitive->setup();
        if (_scene->addPrimitive(_heMeshHighlightShader, linePrimitive) == Common::Return::Succeed)
        {
            _heMeshHighlightPrimitives.pushBack(linePrimitive);
        }
    }
} // namespace EgLab::Platform
