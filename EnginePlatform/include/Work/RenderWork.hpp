#pragma once

#include <atomic>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

#include "Command/CommandBus.hpp"
#include "Command/CommandIds.hpp"
#include "Command/CommandParam.hpp"
#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Singleton.hpp"
#include "Common/Vector.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"

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
} // namespace EgLab::RE

namespace EgLab::Platform
{

    struct RenderPacket
    {
        Common::SharedPtr<RE::RenderPrimitive> renderNode;
        Common::SharedPtr<RE::RenderPrimitive> renderLine;
        Common::SharedPtr<RE::RenderPrimitive> renderFace;
        Common::SharedPtr<ME::Mesh> sourceMesh;
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

        void fitView();

        // RenderWork is a dedicated render event listener.
        void subscribe(EventId eventId);
        void onUpdateMesh(const Common::SharedPtr<EgLab::ME::Mesh>& mesh);
        void onUpdateMesh(const UpdateMeshParam& params);
        void setHighlightedMesh(const Common::SharedPtr<EgLab::ME::Mesh>& mesh);
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

    private:
        struct MeshColor
        {
            Common::SharedPtr<ME::Mesh> mesh;
            Common::Vector4f color;
        };

        std::thread _thread;
        std::atomic<bool> _running{false};
        Common::SharedPtr<EgLab::RE::Window> _window;
        Common::SharedPtr<EgLab::RE::Renderer> _renderer;
        Common::SharedPtr<EgLab::RE::Scene> _scene;
        Common::SharedPtr<EgLab::RE::Camera> _camera;

        std::mutex _renderEventMutex;
        std::deque<RenderEvent> _renderEventQueue;

        Common::DynamicArray<MeshColor> _meshColors;
        Common::DynamicArray<RenderPacket> _activeMeshPackets;

        Common::DynamicArray<ME::Mesh*> _removedMeshes;

        Common::SharedPtr<EgLab::ME::Mesh> _requestedHighlightedMesh;
        Common::SharedPtr<EgLab::ME::Mesh> _activeHighlightedMesh;
        Common::SharedPtr<RE::Shader> _highlightShader;
        Common::SharedPtr<RE::RenderPrimitive> _highlightPrimitive;

        friend class RenderUpdate;

        EgLab::RE::DeltaTime lastFrame;
        EgLab::RE::DeltaTime deltaTime;

        CommandBus _renderUpdateBus{1};

        friend class EgLab::Common::Singleton<RenderWork>;
        friend class RenderUpdate;
        friend class RenderEventCommand;
    };
} // namespace EgLab::Platform
