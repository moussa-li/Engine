#pragma once

#include <atomic>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>
#include <vector>

#include "Command/CommandBus.hpp"
#include "Command/CommandIds.hpp"
#include "Command/CommandParam.hpp"
#include "Common/DynamicArray.hpp"
#include "Common/Publisher.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Singleton.hpp"
#include "Common/Subscriber.hpp"
#include "Common/Vector.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"
#include "RenderEngine/Core/Selector.hpp"

namespace EgLab::ME
{
    class Mesh;
}

namespace EgLab::RE
{
    class Window;
    class Renderer;
    class Scene;
    class Camera;
    class Shader;
    class MeshToTriFace;
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

    struct RenderPacket
    {
        Common::SharedPtr<RE::RenderPrimitive> renderNode;
        Common::SharedPtr<RE::RenderPrimitive> renderLine;
        Common::SharedPtr<RE::RenderPrimitive> renderFace;
        Common::SharedPtr<ME::Mesh> sourceMesh;
        Common::SharedPtr<ME::Mesh> mesh;
        Common::SharedPtr<RE::MeshToTriFace> translator;
    };

    class RenderWork : public EgLab::Common::Singleton<RenderWork>
    {
    public:
        RenderWork();
        ~RenderWork();

        void start();
        void stop();
        void bindWindow(const Common::SharedPtr<EgLab::RE::Window>& window);
        void bindRenderer(const Common::SharedPtr<EgLab::RE::Renderer>& renderer);
        void bindScene(const Common::SharedPtr<EgLab::RE::Scene>& scene);
        void bindCamera(const Common::SharedPtr<EgLab::RE::Camera>& camera);
        PickInfoSubscriptionId subscribePickInfo();
        void unsubscribePickInfo(PickInfoSubscriptionId subscriptionId);
        bool tryReceivePickInfo(PickInfoSubscriptionId subscriptionId, PickedMeshInfo& info);

        void fitView();

        // RenderWork is a dedicated render event listener.
        void subscribe(EventId eventId);
        void onUpdateMesh(const Common::SharedPtr<EgLab::ME::Mesh>& mesh);
        void onUpdateMesh(const UpdateMeshParam& params);
        void setHighlightedMesh(const Common::SharedPtr<EgLab::ME::Mesh>& mesh);
        void setHighlightedMeshes(
            const Common::DynamicArray<Common::SharedPtr<EgLab::ME::Mesh>>& meshes);
        void setMeshColor(const Common::SharedPtr<EgLab::ME::Mesh>& mesh,
                          const Common::Vector4f& color);
        void removeMesh(const Common::SharedPtr<EgLab::ME::Mesh>& mesh);

    private:
        enum class RenderEventType
        {
            MeshUpdate,
            RenderPacket,
            MeshColor,
            MeshRemoval,
            Highlight,
        };

        struct RenderEvent
        {
            RenderEventType type{RenderEventType::MeshUpdate};
            std::vector<uint8_t> payload;
            Common::SharedPtr<ME::Mesh> mesh;
            Common::DynamicArray<Common::SharedPtr<ME::Mesh>> meshes;
            Common::Vector4f color;
            RenderPacket packet;
        };

        void queueRenderEvent(RenderEvent event);
        void enqueueRenderEvent(RenderEvent event);
        void queueMeshUpdate(const Common::SharedPtr<EgLab::ME::Mesh>& mesh);
        void queueMeshUpdate(const UpdateMeshParam& params,
                             const Common::SharedPtr<EgLab::ME::Mesh>& sourceMesh = nullptr);
        void drainRenderEvents();
        bool isMeshRemoved(const ME::Mesh* mesh);
        void materializeMesh(const UpdateMeshParam& params,
                             const Common::SharedPtr<EgLab::ME::Mesh>& sourceMesh);
        void updateMeshHighlight();
        void publishPickInfo(const RE::PrimitivePickInfo& info);

    private:
        struct MeshColor
        {
            Common::SharedPtr<ME::Mesh> mesh;
            Common::Vector4f color;
        };

        struct PickInfoSubscription
        {
            PickInfoSubscriptionId id{0};
            std::unique_ptr<Common::MemoryQueue> queue;
            std::unique_ptr<Common::Publisher> publisher;
            std::unique_ptr<Common::Subscriber> subscriber;
        };

        std::thread _thread;
        std::atomic<bool> _running{false};
        Common::SharedPtr<EgLab::RE::Window> _window;
        Common::SharedPtr<EgLab::RE::Renderer> _renderer;
        Common::SharedPtr<EgLab::RE::Scene> _scene;
        Common::SharedPtr<EgLab::RE::Camera> _camera;
        Common::SharedPtr<EgLab::RE::MeshSelector> _meshSelector;

        std::mutex _pickInfoSubscriptionsMutex;
        std::vector<std::unique_ptr<PickInfoSubscription>> _pickInfoSubscriptions;
        PickInfoSubscriptionId _nextPickInfoSubscriptionId{1};
        PickedMeshInfo _lastPublishedPickInfo;
        bool _hasPublishedPickInfo{false};

        std::mutex _renderEventMutex;
        std::deque<RenderEvent> _renderEventQueue;

        Common::DynamicArray<MeshColor> _meshColors;
        Common::DynamicArray<RenderPacket> _activeMeshPackets;

        Common::DynamicArray<ME::Mesh*> _removedMeshes;

        Common::DynamicArray<Common::SharedPtr<EgLab::ME::Mesh>> _requestedHighlightedMeshes;
        Common::DynamicArray<Common::SharedPtr<EgLab::ME::Mesh>> _activeHighlightedMeshes;
        Common::SharedPtr<RE::Shader> _highlightShader;
        Common::DynamicArray<Common::SharedPtr<RE::RenderPrimitive>> _highlightPrimitives;

        friend class RenderUpdate;

        EgLab::RE::DeltaTime lastFrame;
        EgLab::RE::DeltaTime deltaTime;

        CommandBus _renderUpdateBus{1};

        friend class EgLab::Common::Singleton<RenderWork>;
        friend class RenderUpdate;
        friend class RenderEventCommand;
    };
} // namespace EgLab::Platform
