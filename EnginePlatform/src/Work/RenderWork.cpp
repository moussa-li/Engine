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
            _renderThreadId.store(std::this_thread::get_id());
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
                publishSelectionInfoIfChanged();
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
            _geometrySelector = Common::makeShared<GeometryMeshSelector>();
            _meshSelector = _geometrySelector;
            _meshSelector->registerPickInfoCallback(
                [this](const RE::PrimitivePickInfo& info) { publishPickInfo(info); });
            _renderData.bindMeshSelector(_meshSelector);
            _renderData.bindRenderer(_renderer);
            _renderData.bindVisibilityFilter(
                [this](const Common::SharedPtr<RE::RenderPrimitive>& primitive) {
                    return _renderer != nullptr && _renderer->isPrimitivePickable(primitive);
                });
        }
        else
        {
            _geometrySelector = nullptr;
            _meshSelector = nullptr;
            _renderData.bindMeshSelector(nullptr);
            _renderData.bindRenderer(nullptr);
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
        _renderThreadId.store(std::thread::id());
        _renderEventBus.stop(true);
    }

    void RenderWork::releaseResources()
    {
        stop();

        _geometrySelector = nullptr;
        _meshSelector = nullptr;
        _renderData.releaseResources();

        _selectionInfoSubscriptions.clear();
        _lastPublishedSelectionRevision = 0;
        _hasPublishedSelectionInfo = false;
        _selectionScopeMesh = nullptr;

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
        }
        // Keep producers from recording further events until the render loop has applied the
        // pending batch. The gate is only ever closed while the loop runs: a dead render loop
        // would never open it again and every producer would block forever.
        if (_renderEventsPending.load(std::memory_order_acquire) && _running.load())
        {
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

    SelectionChangedSubscriptionId RenderWork::subscribeSelectionChanged()
    {
        const SelectionChangedSubscriptionId subscriptionId = _nextSelectionInfoSubscriptionId++;

        Common::String queueName = Common::String("RenderSelectionInfo_") +
                                   Common::ToString(reinterpret_cast<uintptr_t>(this)) +
                                   Common::String("_") + Common::ToString(subscriptionId);

        auto subscription = Common::makeUnique<SelectionInfoSubscription>();
        subscription->id = subscriptionId;
        subscription->queue = Common::makeUnique<Common::MemoryQueue>(queueName, true);
        subscription->publisher = Common::makeUnique<Common::Publisher>(*subscription->queue);
        subscription->subscriber = Common::makeUnique<Common::Subscriber>(*subscription->queue);
        subscription->subscriber->subscrib(static_cast<uint32_t>(EventId::SelectionChanged));

        if (_hasPublishedSelectionInfo)
        {
            subscription->publisher->publish(static_cast<uint32_t>(EventId::SelectionChanged),
                                             _lastSelectionChangedInfo);
        }
        _selectionInfoSubscriptions.pushBack(Common::move(subscription));
        return subscriptionId;
    }

    void RenderWork::unsubscribeSelectionChanged(SelectionChangedSubscriptionId subscriptionId)
    {
        Common::DynamicArray<Common::UniquePtr<SelectionInfoSubscription>> remaining;
        for (auto& subscription : _selectionInfoSubscriptions)
        {
            if (subscription->id != subscriptionId)
            {
                remaining.pushBack(Common::move(subscription));
            }
        }
        _selectionInfoSubscriptions.clear();
        _selectionInfoSubscriptions = Common::move(remaining);
    }

    bool RenderWork::tryReceiveSelectionChanged(SelectionChangedSubscriptionId subscriptionId,
                                                SelectionChangedInfo& info)
    {
        for (const auto& subscription : _selectionInfoSubscriptions)
        {
            if (subscription->id == subscriptionId)
            {
                uint32_t eventId = 0;
                return subscription->subscriber->tryReceive(eventId, info);
            }
        }
        return false;
    }

    void RenderWork::publishSelectionInfoIfChanged()
    {
        uint64_t revision = 0;
        {
            std::lock_guard<std::mutex> lock(_renderData.stateMutex());
            revision = _renderData.getSelectionRevision();
        }
        if (revision == _lastPublishedSelectionRevision)
        {
            return;
        }

        SelectionSnapshot snapshot;
        {
            std::lock_guard<std::mutex> lock(_renderData.stateMutex());
            _renderData.getSelectionSnapshot(snapshot);
        }

        SelectionChangedInfo info;
        info.revision = snapshot.revision;
        info.nodeCount = snapshot.totalNodes;
        info.elementCount = snapshot.totalElements;
        _lastSelectionChangedInfo = info;
        _lastPublishedSelectionRevision = revision;
        _hasPublishedSelectionInfo = true;

        for (const auto& subscription : _selectionInfoSubscriptions)
        {
            subscription->publisher->publish(static_cast<uint32_t>(EventId::SelectionChanged),
                                             info);
        }
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

    bool RenderWork::tryGetNearestMeshCenter(float cursorX, float cursorY,
                                             RE::CoordType& center) const
    {
        if (!_window)
        {
            return false;
        }

        unsigned int framebufferWidth = 0;
        unsigned int framebufferHeight = 0;
        unsigned int windowWidth = 0;
        unsigned int windowHeight = 0;
        _window->getFramebufferSize(framebufferWidth, framebufferHeight);
        _window->getWindowSize(windowWidth, windowHeight);
        if (framebufferWidth == 0 || framebufferHeight == 0 || windowWidth == 0 ||
            windowHeight == 0)
        {
            return false;
        }

        // Cursor positions arrive in window coordinates while the camera projection uses
        // framebuffer pixels, so convert before comparing projected centers.
        const float scaleX = static_cast<float>(framebufferWidth) / static_cast<float>(windowWidth);
        const float scaleY =
            static_cast<float>(framebufferHeight) / static_cast<float>(windowHeight);

        std::lock_guard<std::mutex> stateLock(_renderData.stateMutex());
        return _renderData.getNearestMeshCenter(cursorX * scaleX, cursorY * scaleY, center);
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
        pickedInfo.meshId =
            hasPickedMesh ? reinterpret_cast<uint64_t>(pickedMesh.get()) : 0;

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
        if (!hasCursor)
        {
            // No motion this frame: reuse the last known position so a pick-kind change
            // (panel opened, filter toggled) is still evaluated under the stationary cursor
            // instead of the window origin.
            _window->tryGetCursorPos(cursorX, cursorY);
        }

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

    void RenderWork::queueRenderEvent(Common::SharedPtr<RenderEvent> event)
    {
        auto command = Common::SharedPtr<ICommand>(
            Common::makeShared<RenderEventCommand>(this, Common::move(event)));
        // The render loop is the only thread that drains the pending batch and re-opens the
        // push gate, so it must never block on that gate: waiting here would stall the single
        // thread able to release it and deadlock both sides. Render-thread requests are simply
        // appended after the pending batch and applied by the next drain, which preserves the
        // FIFO order of RenderDataManager handlers.
        const bool isRenderThread =
            _renderThreadId.load() == std::this_thread::get_id();
        const bool waitForPushAllowed = !isRenderThread && _running.load();
        if (_renderEventBus.dispatch(command, waitForPushAllowed) != Common::Return::Succeed)
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

    void RenderWork::setCamera(const SetCameraParam& params)
    {
        auto event = Common::makeShared<SetCameraEvent>();
        event->param = params;
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

    void RenderWork::setNodeVisibility(const Common::SharedPtr<EgLab::ME::Mesh>& mesh,
                                       uint32_t mode,
                                       const Common::DynamicArray<uint32_t>& nodeIds)
    {
        if (!mesh)
        {
            LOG(WARNING) << "RenderWork::setNodeVisibility() skipped null mesh";
            return;
        }

        auto event = Common::makeShared<NodeVisibilityEvent>();
        event->mesh = mesh;
        event->nodeIds = nodeIds;
        event->mode = mode;
        queueRenderEvent(event);
    }

    void RenderWork::setSelectionScope(const Common::SharedPtr<EgLab::ME::Mesh>& mesh)
    {
        _selectionScopeMesh = mesh;
    }

    void RenderWork::setSelectionContainsOnly(bool containsOnly)
    {
        _selectionContainsOnly = containsOnly;
    }

    bool RenderWork::isSelectionContainsOnly() const
    {
        return _selectionContainsOnly;
    }

    void RenderWork::requestBoxSelection(float minX, float minY, float maxX, float maxY,
                                         unsigned int width, unsigned int height, bool additive,
                                         uint32_t kinds)
    {
        auto event = Common::makeShared<BoxSelectEvent>();
        event->minX = minX;
        event->minY = minY;
        event->maxX = maxX;
        event->maxY = maxY;
        event->width = width;
        event->height = height;
        event->containsOnly = _selectionContainsOnly;
        event->additive = additive;
        event->kinds = kinds;
        event->scopeMesh = _selectionScopeMesh;
        queueRenderEvent(event);
    }

    void RenderWork::requestPointSelection(float x, float y, unsigned int width,
                                           unsigned int height, bool additive, uint32_t kinds)
    {
        auto event = Common::makeShared<PointSelectEvent>();
        event->x = x;
        event->y = y;
        event->width = width;
        event->height = height;
        event->additive = additive;
        event->kinds = kinds;
        event->scopeMesh = _selectionScopeMesh;
        queueRenderEvent(event);
    }

    void RenderWork::requestClearSelection()
    {
        queueRenderEvent(Common::makeShared<ClearSelectionEvent>());
    }

    void RenderWork::copySelectionSnapshot(SelectionSnapshot& snapshot) const
    {
        std::lock_guard<std::mutex> lock(_renderData.stateMutex());
        _renderData.getSelectionSnapshot(snapshot);
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

    void RenderWork::setGeometryDisplayMode(uint32_t mode)
    {
        if (_geometrySelector)
        {
            // Face outline and edges-only draw B-Rep edge segments, so line picks resolve to
            // Edge features instead of the triangle wireframe's face elements.
            _geometrySelector->setBrepEdgeLines(mode != 0);
        }
        auto event = Common::makeShared<GeometryDisplayModeEvent>();
        event->mode = mode;
        queueRenderEvent(event);
    }

    void RenderWork::setGeometryVertexDisplay(bool visible)
    {
        auto event = Common::makeShared<GeometryVertexDisplayEvent>();
        event->visible = visible;
        queueRenderEvent(event);
    }

    bool RenderWork::getGeometryVertexDisplay() const
    {
        return _renderData.getGeometryVertexDisplay();
    }

    void RenderWork::setGeometryPickFilter(uint32_t mask)
    {
        // The selector stores the mask atomically, so picking on the render thread always sees
        // a complete value and no render event round trip is needed.
        if (_geometrySelector)
        {
            _geometrySelector->setFeatureTypeMask(mask);
        }
    }

    uint32_t RenderWork::getGeometryPickFilter() const
    {
        return _geometrySelector ? _geometrySelector->getFeatureTypeMask()
                                 : GEOMETRY_FEATURE_MASK_ALL;
    }

    void RenderWork::setReferencePlaneSettings(const SetReferencePlaneParam& params)
    {
        auto event = Common::makeShared<ReferencePlaneSettingsEvent>();
        event->param = params;
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