#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <thread>
#include <type_traits>

#include "Command/CommandBus.hpp"
#include "Command/CommandIds.hpp"
#include "Command/CommandParam.hpp"
#include "Common/DynamicArray.hpp"
#include "Common/Publisher.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Singleton.hpp"
#include "Common/Subscriber.hpp"
#include "Common/UniquePtr.hpp"
#include "Common/Vector.hpp"
#include "DataBase/GeometryMeshSelector.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/PickManager.hpp"
#include "RenderEngine/Core/Selector.hpp"
#include "Work/RenderDataManager.hpp"
#include "Work/RenderEvent.hpp"
#include "Work/RenderEventHandler.hpp"
#include "Work/SelectionModel.hpp"

namespace EgLab::ME
{
    class Mesh;
    class HEMesh;
} // namespace EgLab::ME

namespace EgLab::RE
{
    class Window;
    class Renderer;
    class Scene;
    class Camera;
    class MeshSelector;
} // namespace EgLab::RE

namespace EgLab::Platform
{
    using PickInfoSubscriptionId = uint64_t;
    using SelectionChangedSubscriptionId = uint64_t;

    /// Selection counts published to panels after a selection change. The payload is POD so it
    /// can travel through Common::MemoryQueue without blocking the render thread.
    struct SelectionChangedInfo
    {
        uint64_t revision{0};
        uint64_t nodeCount{0};
        uint64_t elementCount{0};
    };

    static_assert(std::is_trivially_copyable<SelectionChangedInfo>::value,
                  "SelectionChangedInfo must be trivially copyable");

    enum class PickedMeshKind : uint32_t
    {
        None,
        Node,
        Element,
        ElementEdge,
        ElementFace
    };

    struct PickedMeshInfo
    {
        PickedMeshKind kind{PickedMeshKind::None};
        uint32_t entityId{UINT32_MAX};
        uint32_t faceIndex{UINT32_MAX};
        uint32_t edgeIndex{UINT32_MAX};
        double x{0.0};
        double y{0.0};
        double z{0.0};
        uint32_t topologyNodeCount{0};
        uint32_t topologyNodeIds[20]{};
        // 拾取到的 ME::Mesh 对象指针值，用于反查 GeometryData；0 表示无。
        uint64_t meshId{0};
    };

    static_assert(std::is_trivially_copyable<PickedMeshInfo>::value,
                  "PickedMeshInfo must be trivially copyable");
    static_assert(sizeof(PickedMeshInfo) == 136,
                  "PickedMeshInfo must be exactly 136 bytes");

    /// Accepts rendering requests, forwards them to their RenderEvent handlers and runs the
    /// render loop. RenderWork does not own or manage render data; all render state lives in
    /// RenderDataManager.
    class RenderWork : public EgLab::Common::Singleton<RenderWork>
    {
    public:
        RenderWork();
        ~RenderWork();

        void start();
        void stop();
        void releaseResources();
        Common::Return commitPendingRenderUpdates();
        void bindWindow(const Common::SharedPtr<EgLab::RE::Window>& window);
        void bindRenderer(const Common::SharedPtr<EgLab::RE::Renderer>& renderer);
        void bindScene(const Common::SharedPtr<EgLab::RE::Scene>& scene);
        void bindCamera(const Common::SharedPtr<EgLab::RE::Camera>& camera);
        PickInfoSubscriptionId subscribePickInfo();
        void unsubscribePickInfo(PickInfoSubscriptionId subscriptionId);
        bool tryReceivePickInfo(PickInfoSubscriptionId subscriptionId, PickedMeshInfo& info);
        // Returns the world-space center of the most recently picked mesh.
        bool tryGetPickedMeshCenter(RE::CoordType& center) const;
        // Returns the world-space bounding-box center of the mesh whose projected center is
        // nearest to the window-space cursor. Does not require a geometry pick.
        bool tryGetNearestMeshCenter(float cursorX, float cursorY, RE::CoordType& center) const;

        // Pick registry shared by the renderer and the features that need picking.
        RE::PickManager& pickManager()
        {
            return _pickManager;
        }
        const RE::PickManager& pickManager() const
        {
            return _pickManager;
        }

        // Queues one pick query at a window-space cursor position. Used by features that need
        // a pick even when no other feature acquired picking, such as the camera pivot.
        void requestPick(float cursorX, float cursorY, uint32_t kinds);

        void fitView();
        void fitView(const Common::SharedPtr<EgLab::ME::Mesh>& mesh);
        void setCamera(const SetCameraParam& params);

        // RenderWork is a dedicated render event listener.
        void subscribe(EventId eventId);
        void onUpdateMesh(const Common::SharedPtr<EgLab::ME::Mesh>& mesh);
        void onUpdateMesh(const UpdateMeshParam& params);
        void setHighlightedMesh(const Common::SharedPtr<EgLab::ME::Mesh>& mesh);
        void setHighlightedMeshes(
            const Common::DynamicArray<Common::SharedPtr<EgLab::ME::Mesh>>& meshes);
        // Highlights the given elements of one mesh; every other face of that mesh becomes
        // transparent. Pass an empty element list to clear the locate state.
        void locateElements(const Common::SharedPtr<EgLab::ME::Mesh>& mesh,
                            const Common::DynamicArray<uint32_t>& elementIds);
        void clearLocate();
        // Changes element visibility of one mesh. mode: 0 = hide ids, 1 = show ids,
        // 2 = show only ids (hide the rest), 3 = hide only ids (show the rest). An empty id
        // list in modes 1/3 restores the full mesh surface.
        void setElementsVisibility(const Common::SharedPtr<EgLab::ME::Mesh>& mesh,
                                   uint32_t mode,
                                   const Common::DynamicArray<uint32_t>& elementIds);
        // Changes node visibility of one mesh with the same mode semantics as elements.
        void setNodeVisibility(const Common::SharedPtr<EgLab::ME::Mesh>& mesh, uint32_t mode,
                               const Common::DynamicArray<uint32_t>& nodeIds);

        // ---- rubber-band selection -------------------------------------------------------
        // These settings are render-thread confined: the UI panel and the render loop run on
        // the same thread in EnginePlatform.
        void setSelectionScope(const Common::SharedPtr<EgLab::ME::Mesh>& mesh);
        void setSelectionContainsOnly(bool containsOnly);
        bool isSelectionContainsOnly() const;
        // Queues a rectangle selection. Coordinates are framebuffer pixels with a top-left
        // origin; the render loop resolves entities and rebuilds the selection highlight.
        void requestBoxSelection(float minX, float minY, float maxX, float maxY,
                                 unsigned int width, unsigned int height, bool additive,
                                 uint32_t kinds);
        // Queues a single point selection at a framebuffer cursor position.
        void requestPointSelection(float x, float y, unsigned int width, unsigned int height,
                                   bool additive, uint32_t kinds);
        void requestClearSelection();
        void copySelectionSnapshot(SelectionSnapshot& snapshot) const;

        // Selection change notifications, consumed by the ElementVisibilityPanel.
        SelectionChangedSubscriptionId subscribeSelectionChanged();
        void unsubscribeSelectionChanged(SelectionChangedSubscriptionId subscriptionId);
        bool tryReceiveSelectionChanged(SelectionChangedSubscriptionId subscriptionId,
                                        SelectionChangedInfo& info);
        void setHighlightColor(const Common::Vector4f& color);
        void setNodePointSize(float pointSize);
        void setLineWidth(float lineWidth);
        void setFaceShrink(float shrink);
        // Switches how imported B-Rep geometry is drawn: 0=mesh, 1=face outline, 2=edges only.
        void setGeometryDisplayMode(uint32_t mode);
        // Shows or hides the B-Rep vertex markers of imported geometry models.
        void setGeometryVertexDisplay(bool visible);
        bool getGeometryVertexDisplay() const;
        // Turns the zebra (isophote) stripe analysis of geometry faces on or off.
        // lightDirection selects the virtual light the stripes mirror instead of the camera.
        void setGeometryZebra(bool enabled, float frequency,
                              const Common::Vector3f& lightDirection);
        // Shows the surface UV of geometry faces, coloured by the interpolated UV.
        void setGeometryUv(bool enabled);
        // Restricts which B-Rep feature types may be picked/selected (GEOMETRY_FEATURE_MASK_*).
        void setGeometryPickFilter(uint32_t mask);
        uint32_t getGeometryPickFilter() const;
        // Applies a partial reference-plane update on the render thread.
        void setReferencePlaneSettings(const SetReferencePlaneParam& params);
        void setHEMeshHighlight(const Common::SharedPtr<ME::HEMesh>& mesh, uint32_t vertexIndex,
                                uint32_t halfEdgeIndex = UINT32_MAX);
        void setMeshColor(const Common::SharedPtr<EgLab::ME::Mesh>& mesh,
                          const Common::Vector4f& color);
        void removeMesh(const Common::SharedPtr<EgLab::ME::Mesh>& mesh);

    private:
        void queueRenderEvent(Common::SharedPtr<RenderEvent> event);
        void drainRenderEvents();
        void processRenderEvent(Common::SharedPtr<RenderEvent> event);
        void publishPickInfo(const RE::PrimitivePickInfo& info);
        // Picking step of the render loop: turns cursor/feature requests into pick events.
        void submitPickEvent();
        void queuePickEvent(float framebufferX, float framebufferY, uint32_t kinds);
        void publishSelectionInfoIfChanged();

    private:
        struct PendingPick
        {
            float x{0.0f};
            float y{0.0f};
            uint32_t kinds{0};
        };

    private:
        struct PickInfoSubscription
        {
            PickInfoSubscriptionId id{0};
            Common::UniquePtr<Common::MemoryQueue> queue;
            Common::UniquePtr<Common::Publisher> publisher;
            Common::UniquePtr<Common::Subscriber> subscriber;
        };

        struct SelectionInfoSubscription
        {
            SelectionChangedSubscriptionId id{0};
            Common::UniquePtr<Common::MemoryQueue> queue;
            Common::UniquePtr<Common::Publisher> publisher;
            Common::UniquePtr<Common::Subscriber> subscriber;
        };

        std::thread _thread;
        std::atomic<bool> _running{false};
        Common::SharedPtr<EgLab::RE::Window> _window;
        Common::SharedPtr<EgLab::RE::Renderer> _renderer;
        Common::SharedPtr<EgLab::RE::Scene> _scene;
        Common::SharedPtr<EgLab::RE::Camera> _camera;
        Common::SharedPtr<EgLab::RE::MeshSelector> _meshSelector;
        Common::SharedPtr<GeometryMeshSelector> _geometrySelector;
        RE::PickManager _pickManager;
        std::mutex _pendingPickMutex;
        PendingPick _pendingPick;
        bool _hasPendingPick{false};
        uint32_t _lastQueuedPickKinds{0};

        mutable std::mutex _pickInfoSubscriptionsMutex;
        Common::DynamicArray<Common::UniquePtr<PickInfoSubscription>> _pickInfoSubscriptions;
        PickInfoSubscriptionId _nextPickInfoSubscriptionId{1};
        PickedMeshInfo _lastPublishedPickInfo;
        bool _hasPublishedPickInfo{false};
        Common::SharedPtr<ME::Mesh> _pickedMesh;
        bool _hasPickedMesh{false};

        Common::DynamicArray<Common::UniquePtr<SelectionInfoSubscription>>
            _selectionInfoSubscriptions;
        SelectionChangedSubscriptionId _nextSelectionInfoSubscriptionId{1};
        SelectionChangedInfo _lastSelectionChangedInfo;
        uint64_t _lastPublishedSelectionRevision{0};
        bool _hasPublishedSelectionInfo{false};
        Common::SharedPtr<ME::Mesh> _selectionScopeMesh;
        bool _selectionContainsOnly{true};

        std::mutex _renderEventMutex;
        Common::DynamicArray<Common::SharedPtr<RenderEvent>> _renderEventQueue;
        std::atomic<bool> _renderEventsPending{false};
        // Identity of the render loop thread: the only thread that drains the pending batch
        // and therefore the only one allowed to release the render event push gate.
        std::atomic<std::thread::id> _renderThreadId{};

        RenderDataManager _renderData;
        RenderEventHandlerFactory _handlerFactory;

        EgLab::RE::DeltaTime lastFrame;
        EgLab::RE::DeltaTime deltaTime;

        CommandBus _renderEventBus{1};

        friend class EgLab::Common::Singleton<RenderWork>;
        friend class RenderEventCommand;
    };
} // namespace EgLab::Platform
