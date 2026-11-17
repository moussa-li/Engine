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
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/PickManager.hpp"
#include "RenderEngine/Core/Selector.hpp"
#include "Work/RenderDataManager.hpp"
#include "Work/RenderEvent.hpp"
#include "Work/RenderEventHandler.hpp"

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
    };

    static_assert(std::is_trivially_copyable<PickedMeshInfo>::value,
                  "PickedMeshInfo must be trivially copyable");

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

        // Pick registry shared by the renderer and the features that need picking.
        RE::PickManager& pickManager()
        {
            return _pickManager;
        }
        const RE::PickManager& pickManager() const
        {
            return _pickManager;
        }

        void fitView();
        void fitView(const Common::SharedPtr<EgLab::ME::Mesh>& mesh);

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
        void setHighlightColor(const Common::Vector4f& color);
        void setNodePointSize(float pointSize);
        void setLineWidth(float lineWidth);
        void setFaceShrink(float shrink);
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

    private:
        struct PickInfoSubscription
        {
            PickInfoSubscriptionId id{0};
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
        RE::PickManager _pickManager;

        mutable std::mutex _pickInfoSubscriptionsMutex;
        Common::DynamicArray<Common::UniquePtr<PickInfoSubscription>> _pickInfoSubscriptions;
        PickInfoSubscriptionId _nextPickInfoSubscriptionId{1};
        PickedMeshInfo _lastPublishedPickInfo;
        bool _hasPublishedPickInfo{false};
        Common::SharedPtr<ME::Mesh> _pickedMesh;
        bool _hasPickedMesh{false};

        std::mutex _renderEventMutex;
        Common::DynamicArray<Common::SharedPtr<RenderEvent>> _renderEventQueue;
        std::atomic<bool> _renderEventsPending{false};

        RenderDataManager _renderData;
        RenderEventHandlerFactory _handlerFactory;

        EgLab::RE::DeltaTime lastFrame;
        EgLab::RE::DeltaTime deltaTime;

        CommandBus _renderEventBus{1};

        friend class EgLab::Common::Singleton<RenderWork>;
        friend class RenderEventCommand;
    };
} // namespace EgLab::Platform
