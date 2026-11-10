#pragma once

#include <atomic>
#include <cstdint>
#include <deque>
#include <memory>
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
#include "Common/Vector.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"
#include "RenderEngine/Core/Selector.hpp"

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
        void releaseResources();
        Common::Return commitPendingRenderUpdates();
        void bindWindow(const Common::SharedPtr<EgLab::RE::Window>& window);
        void bindRenderer(const Common::SharedPtr<EgLab::RE::Renderer>& renderer);
        void bindScene(const Common::SharedPtr<EgLab::RE::Scene>& scene);
        void bindCamera(const Common::SharedPtr<EgLab::RE::Camera>& camera);
        PickInfoSubscriptionId subscribePickInfo();
        void unsubscribePickInfo(PickInfoSubscriptionId subscriptionId);
        bool tryReceivePickInfo(PickInfoSubscriptionId subscriptionId, PickedMeshInfo& info);

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
        enum class RenderEventType
        {
            MeshColor,
            MeshUpdate,
            MeshRemoval,
            Highlight,
            HighlightColor,
            NodePointSize,
            LineWidth,
            FaceShrink,
            HEMeshHighlight,
            Locate,
            FitView,
        };

        struct RenderEvent
        {
            RenderEventType type{RenderEventType::FitView};
            Common::SharedPtr<ME::Mesh> mesh;
            Common::DynamicArray<Common::SharedPtr<ME::Mesh>> meshes;
            Common::SharedPtr<ME::HEMesh> heMesh;
            uint32_t heMeshVertexIndex{UINT32_MAX};
            uint32_t heMeshHalfEdgeIndex{UINT32_MAX};
            Common::DynamicArray<uint32_t> elementIds;
            Common::Vector4f color;
            float nodePointSize{3.0f};
            float lineWidth{2.0f};
            float faceShrink{0.0f};
            Common::DynamicArray<uint8_t> payload;
            RenderPacket packet;
        };

        void queueRenderEvent(RenderEvent event);
        void drainRenderEvents();
        void processRenderEvent(RenderEvent event);
        Common::Return applyRenderUpdate(const RenderEvent& event);
        bool isMeshRemoved(const ME::Mesh* mesh);
        Common::Return uploadRenderPacket(const RenderPacket& packet);
        void updateLocateHighlight();
        Common::Vector4f getMeshBaseColor(const Common::SharedPtr<ME::Mesh>& mesh) const;
        void updateMeshHighlight();
        void updateViewDependentHighlights();
        void updateHEMeshHighlight(const RenderEvent& event);
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
        Common::DynamicArray<std::unique_ptr<PickInfoSubscription>> _pickInfoSubscriptions;
        PickInfoSubscriptionId _nextPickInfoSubscriptionId{1};
        PickedMeshInfo _lastPublishedPickInfo;
        bool _hasPublishedPickInfo{false};

        std::mutex _renderEventMutex;
        std::deque<RenderEvent> _renderEventQueue;
        std::atomic<bool> _renderEventsPending{false};
        std::mutex _renderStateMutex;

        Common::DynamicArray<MeshColor> _meshColors;
        Common::DynamicArray<RenderPacket> _activeMeshPackets;

        Common::DynamicArray<ME::Mesh*> _removedMeshes;

        Common::DynamicArray<Common::SharedPtr<EgLab::ME::Mesh>> _requestedHighlightedMeshes;
        Common::DynamicArray<Common::SharedPtr<EgLab::ME::Mesh>> _activeHighlightedMeshes;
        Common::SharedPtr<RE::Shader> _highlightShader;
        Common::DynamicArray<Common::SharedPtr<RE::RenderPrimitive>> _highlightPrimitives;
        // Locate state: the target mesh, the requested element ids and the primitives that draw
        // the located elements. The target mesh faces are dimmed while a locate is active.
        Common::SharedPtr<ME::Mesh> _locateMesh;
        Common::DynamicArray<uint32_t> _locateElementIds;
        Common::DynamicArray<Common::SharedPtr<RE::RenderPrimitive>> _locatePrimitives;
        Common::SharedPtr<RE::RenderPrimitive> _locateTargetFace;
        Common::SharedPtr<ME::Mesh> _locateTargetMesh;
        Common::Vector4f _locateColor{1.0f, 0.55f, 0.1f, 1.0f};
        Common::DynamicArray<Common::SharedPtr<RE::RenderPrimitive>> _highlightContourPrimitives;
        Common::Vector4f _highlightColor{1.0f, 0.75f, 0.7f, 0.3f};
        float _nodePointSize{3.0f};
        float _lineWidth{2.0f};
        float _faceShrink{0.0f};
        RE::CoordType _highlightViewPosition;
        RE::CoordType _highlightViewFront;
        bool _highlightViewValid{false};
        Common::DynamicArray<Common::SharedPtr<RE::RenderPrimitive>> _heMeshHighlightPrimitives;
        Common::SharedPtr<RE::Shader> _heMeshHighlightShader;

        EgLab::RE::DeltaTime lastFrame;
        EgLab::RE::DeltaTime deltaTime;

        CommandBus _renderEventBus{1};

        friend class EgLab::Common::Singleton<RenderWork>;
        friend class RenderEventCommand;
    };
} // namespace EgLab::Platform
