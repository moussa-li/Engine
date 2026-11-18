#include "Work/RenderWork.hpp"

#include <exception>
#include <mutex>
#include <thread>

#include "Common/Log.hpp"
#include "Common/StringTools.hpp"
#include "RenderEngine/Core/MeshSelector.hpp"
#include "RenderEngine/Core/Renderer.hpp"
#include "RenderEngine/Core/Scene.hpp"
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
        RenderEventCommand(RenderWork* worker, Common::SharedPtr<RenderEvent> event)
            : _worker(worker), _event(Common::move(event))
        {
        }

        Common::Return exec() override
        {
            try
            {
                _worker->processRenderEvent(Common::move(_event));
            }
            catch (const std::exception& exception)
            {
                LOG(ERROR) << "RenderEventCommand::exec() failed: " << exception.what();
                return Common::Return::Failed;
            }
            catch (...)
            {
                LOG(ERROR) << "RenderEventCommand::exec() failed with an unknown exception";
                return Common::Return::Failed;
            }
            return Common::Return::Succeed;
        }

    private:
        RenderWork* _worker;
        Common::SharedPtr<RenderEvent> _event;
    };

    RenderWork::RenderWork() : _handlerFactory(_renderData)
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

        _running = true;
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

                // Picking is part of the render loop: requests become pick events, and the
                // handler exposes the highlight as a scene primitive drawn with the rest.
                submitPickEvent();
                drainRenderEvents();
                if (!_running.load())
                {
                    _window->deactiveContext();
                    break;
                }
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
            _meshSelector->registerPickInfoCallback(
                [this](const RE::PrimitivePickInfo& info) { publishPickInfo(info); });
            _renderData.bindMeshSelector(_meshSelector);
            _renderData.bindVisibilityFilter(
                [this](const Common::SharedPtr<RE::RenderPrimitive>& primitive) {
                    return _renderer != nullptr && _renderer->isPrimitiveVisible(primitive);
                });
        }
        else
        {
            _meshSelector = nullptr;
            _renderData.bindMeshSelector(nullptr);
            _renderData.bindVisibilityFilter(nullptr);
        }
    }

    void RenderWork::bindScene(const Common::SharedPtr<EgLab::RE::Scene>& scene)
    {
        _scene = scene;
        _renderData.bindScene(scene);
    }

    void RenderWork::bindCamera(const Common::SharedPtr<EgLab::RE::Camera>& camera)
    {
        _camera = camera;
        _renderData.bindCamera(camera);
    }

    void RenderWork::stop()
    {
        _running = false;
        _renderEventBus.setPushAllowed(true);
        if (_thread.joinable())
        {
            _thread.join();
        }
        _renderEventBus.stop(true);
    }

    void RenderWork::releaseResources()
    {
        stop();

        _meshSelector = nullptr;
        _renderData.releaseResources();

        {
            std::lock_guard<std::mutex> lock(_renderEventMutex);
            _renderEventQueue.clear();
            _renderEventsPending.store(false, std::memory_order_release);
        }
        _renderEventBus.setPushAllowed(true);

        _scene = nullptr;
        _renderer = nullptr;
        _camera = nullptr;
        _window = nullptr;

        {
            std::lock_guard<std::mutex> lock(_pendingPickMutex);
            _hasPendingPick = false;
        }
        _lastQueuedPickKinds = 0;
    }

    Common::Return RenderWork::commitPendingRenderUpdates()
    {
        _renderEventBus.waitIdle();
        std::lock_guard<std::mutex> lock(_renderEventMutex);
        if (!_renderEventQueue.empty())
        {
            _renderEventsPending.store(true, std::memory_order_release);
            _renderEventBus.setPushAllowed(false);
        }
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

        Common::String queueName = Common::String("RenderPickInfo_") +
                                   Common::ToString(reinterpret_cast<uintptr_t>(this)) +
                                   Common::String("_") + Common::ToString(subscriptionId);

        auto subscription = Common::makeUnique<PickInfoSubscription>();
        subscription->id = subscriptionId;
        subscription->queue = Common::makeUnique<Common::MemoryQueue>(queueName, true);
        subscription->publisher = Common::makeUnique<Common::Publisher>(*subscription->queue);
        subscription->subscriber = Common::makeUnique<Common::Subscriber>(*subscription->queue);
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
        Common::DynamicArray<Common::UniquePtr<PickInfoSubscription>> remaining;
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

    bool RenderWork::tryGetPickedMeshCenter(RE::CoordType& center) const
    {
        Common::SharedPtr<ME::Mesh> pickedMesh;
        {
            std::lock_guard<std::mutex> lock(_pickInfoSubscriptionsMutex);
            if (!_hasPickedMesh)
            {
                return false;
            }
            pickedMesh = _pickedMesh;
        }

        // Compute on demand so the center follows the currently visible primitives.
        std::lock_guard<std::mutex> stateLock(_renderData.stateMutex());
        return _renderData.getMeshCenter(pickedMesh, center);
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

        Common::SharedPtr<ME::Mesh> pickedMesh;
        bool hasPickedMesh = false;
        if (info.kind != RE::PrimitivePickKind::None && _meshSelector)
        {
            hasPickedMesh = _meshSelector->tryGetPickedMesh(pickedMesh);
        }

        std::lock_guard<std::mutex> lock(_pickInfoSubscriptionsMutex);
        _lastPublishedPickInfo = pickedInfo;
        _hasPublishedPickInfo = true;
        _pickedMesh = pickedMesh;
        _hasPickedMesh = hasPickedMesh;
        for (const auto& subscription : _pickInfoSubscriptions)
        {
            subscription->publisher->publish(static_cast<uint32_t>(EventId::MeshPickInfo),
                                             pickedInfo);
        }
    }

    void RenderWork::requestPick(float cursorX, float cursorY, uint32_t kinds)
    {
        if (kinds == 0)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(_pendingPickMutex);
        _pendingPick.x = cursorX;
        _pendingPick.y = cursorY;
        _pendingPick.kinds = kinds;
        _hasPendingPick = true;
    }

    void RenderWork::queuePickEvent(float framebufferX, float framebufferY, uint32_t kinds)
    {
        auto event = Common::makeShared<PickEvent>();
        event->x = framebufferX;
        event->y = framebufferY;
        event->kinds = kinds;
        queueRenderEvent(event);
    }

    void RenderWork::submitPickEvent()
    {
        if (_window == nullptr || _scene == nullptr || _camera == nullptr)
        {
            return;
        }

        float cursorX = 0.0f;
        float cursorY = 0.0f;
        const bool hasCursor = _window->trySelectedPos(cursorX, cursorY);

        PendingPick pending;
        bool hasPending = false;
        {
            std::lock_guard<std::mutex> lock(_pendingPickMutex);
            if (_hasPendingPick)
            {
                pending = _pendingPick;
                _hasPendingPick = false;
                hasPending = true;
            }
        }

        const uint32_t requestedKinds = _pickManager.activeKinds();
        if (!hasCursor && !hasPending && requestedKinds == _lastQueuedPickKinds)
        {
            return;
        }

        const float windowX = hasPending ? pending.x : cursorX;
        const float windowY = hasPending ? pending.y : cursorY;
        const uint32_t kinds = hasPending ? pending.kinds : requestedKinds;

        unsigned int framebufferWidth = 0;
        unsigned int framebufferHeight = 0;
        unsigned int windowWidth = 0;
        unsigned int windowHeight = 0;
        _window->getFramebufferSize(framebufferWidth, framebufferHeight);
        _window->getWindowSize(windowWidth, windowHeight);
        if (framebufferWidth == 0 || framebufferHeight == 0 || windowWidth == 0 ||
            windowHeight == 0)
        {
            return;
        }

        const float scaleX = static_cast<float>(framebufferWidth) / static_cast<float>(windowWidth);
        const float scaleY =
            static_cast<float>(framebufferHeight) / static_cast<float>(windowHeight);

        _lastQueuedPickKinds = kinds;
        queuePickEvent(windowX * scaleX, windowY * scaleY, kinds);
    }

    void RenderWork::queueRenderEvent(Common::SharedPtr<RenderEvent> event)    {
        auto command = Common::SharedPtr<ICommand>(
            Common::makeShared<RenderEventCommand>(this, Common::move(event)));
        if (_renderEventBus.dispatch(command, true) != Common::Return::Succeed)
        {
            LOG(WARNING)
                << "RenderWork::queueRenderEvent() dropped event because its queue is stopped";
        }
    }

    void RenderWork::drainRenderEvents()
    {
        Common::DynamicArray<Common::SharedPtr<RenderEvent>> events;
        {
            std::lock_guard<std::mutex> lock(_renderEventMutex);
            if (!_renderEventsPending.load(std::memory_order_acquire))
            {
                return;
            }
            events = Common::move(_renderEventQueue);
            _renderEventsPending.store(false, std::memory_order_release);
            _renderEventBus.setPushAllowed(true);
        }

        for (size_t index = 0; index < events.size(); ++index)
        {
            if (!_running.load())
            {
                break;
            }
            const Common::SharedPtr<RenderEvent>& event = events[index];
            auto handler = _handlerFactory.create(event->getType());
            if (!handler)
            {
                LOG(ERROR) << "RenderWork::drainRenderEvents() no handler for render event type";
                continue;
            }
            std::lock_guard<std::mutex> lock(_renderData.stateMutex());
            handler->apply(*event);
        }
    }

    void RenderWork::processRenderEvent(Common::SharedPtr<RenderEvent> event)
    {
        auto handler = _handlerFactory.create(event->getType());
        if (!handler)
        {
            LOG(ERROR) << "RenderWork::processRenderEvent() no handler for render event type";
            return;
        }

        if (handler->prepare(*event) != Common::Return::Succeed)
        {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(_renderData.stateMutex());
            if (handler->record(*event) != Common::Return::Succeed)
            {
                return;
            }
        }

        {
            std::lock_guard<std::mutex> lock(_renderEventMutex);
            _renderEventQueue.pushBack(Common::move(event));
            _renderEventsPending.store(true, std::memory_order_release);
        }
    }

    void RenderWork::fitView()
    {
        queueRenderEvent(Common::makeShared<FitViewEvent>());
    }

    void RenderWork::fitView(const Common::SharedPtr<EgLab::ME::Mesh>& mesh)
    {
        auto event = Common::makeShared<FitViewEvent>();
        event->mesh = mesh;
        queueRenderEvent(event);
    }

    void RenderWork::onUpdateMesh(const UpdateMeshParam& params)
    {
        if (params.data == nullptr || params.dataSize == 0)
        {
            return;
        }

        auto event = Common::makeShared<MeshUpdateEvent>();
        event->payload.pushBack(params.data, params.dataSize);
        queueRenderEvent(event);
    }

    void RenderWork::onUpdateMesh(const Common::SharedPtr<EgLab::ME::Mesh>& mesh)
    {
        if (!mesh)
        {
            return;
        }

        auto event = Common::makeShared<MeshUpdateEvent>();
        event->mesh = mesh;
        queueRenderEvent(event);
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
        auto event = Common::makeShared<HighlightEvent>();
        event->meshes = meshes;
        queueRenderEvent(event);
    }

    void RenderWork::locateElements(const Common::SharedPtr<EgLab::ME::Mesh>& mesh,
                                    const Common::DynamicArray<uint32_t>& elementIds)
    {
        auto event = Common::makeShared<LocateEvent>();
        event->mesh = mesh;
        event->elementIds = elementIds;
        queueRenderEvent(event);
    }

    void RenderWork::clearLocate()
    {
        locateElements(Common::SharedPtr<ME::Mesh>(), Common::DynamicArray<uint32_t>());
    }

    void RenderWork::setElementsVisibility(const Common::SharedPtr<EgLab::ME::Mesh>& mesh,
                                           uint32_t mode,
                                           const Common::DynamicArray<uint32_t>& elementIds)
    {
        if (!mesh)
        {
            LOG(WARNING) << "RenderWork::setElementsVisibility() skipped null mesh";
            return;
        }

        auto event = Common::makeShared<ElementsVisibilityEvent>();
        event->mesh = mesh;
        event->elementIds = elementIds;
        event->mode = mode;
        queueRenderEvent(event);
    }

    void RenderWork::setHighlightColor(const Common::Vector4f& color)
    {
        auto event = Common::makeShared<HighlightColorEvent>();
        event->color = color;
        queueRenderEvent(event);
    }

    void RenderWork::setNodePointSize(float pointSize)
    {
        auto event = Common::makeShared<NodePointSizeEvent>();
        event->pointSize = pointSize;
        queueRenderEvent(event);
    }

    void RenderWork::setLineWidth(float lineWidth)
    {
        auto event = Common::makeShared<LineWidthEvent>();
        event->lineWidth = lineWidth;
        queueRenderEvent(event);
    }

    void RenderWork::setFaceShrink(float shrink)
    {
        auto event = Common::makeShared<FaceShrinkEvent>();
        event->faceShrink = shrink;
        queueRenderEvent(event);
    }

    void RenderWork::setHEMeshHighlight(const Common::SharedPtr<ME::HEMesh>& mesh,
                                        uint32_t vertexIndex, uint32_t halfEdgeIndex)
    {
        auto event = Common::makeShared<HEMeshHighlightEvent>();
        event->heMesh = mesh;
        event->vertexIndex = vertexIndex;
        event->halfEdgeIndex = halfEdgeIndex;
        queueRenderEvent(event);
    }

    void RenderWork::setMeshColor(const Common::SharedPtr<EgLab::ME::Mesh>& mesh,
                                  const Common::Vector4f& color)
    {
        if (!mesh)
        {
            LOG(WARNING) << "RenderWork::setMeshColor() skipped null mesh";
            return;
        }

        auto event = Common::makeShared<MeshColorEvent>();
        event->mesh = mesh;
        event->color = color;
        queueRenderEvent(event);
    }

    void RenderWork::removeMesh(const Common::SharedPtr<ME::Mesh>& mesh)
    {
        if (!mesh)
        {
            return;
        }

        auto event = Common::makeShared<MeshRemovalEvent>();
        event->mesh = mesh;
        queueRenderEvent(event);
    }
} // namespace EgLab::Platform