#pragma once

#include <atomic>
#include <mutex>
#include <thread>

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
        // Only EventId::UpdateMesh is expected to enter this lane.
        void subscribe(EventId eventId);
        void onUpdateMesh(const Common::SharedPtr<EgLab::ME::Mesh>& mesh);
        void onUpdateMesh(const UpdateMeshParam& params);
        void setHighlightedMesh(const Common::SharedPtr<EgLab::ME::Mesh>& mesh);
        void setMeshColor(const Common::SharedPtr<EgLab::ME::Mesh>& mesh,
                          const Common::Vector4f& color);

    private:
        void queueMeshUpdate(const Common::SharedPtr<EgLab::ME::Mesh>& mesh);
        void queueMeshUpdate(const UpdateMeshParam& params,
                             const Common::SharedPtr<EgLab::ME::Mesh>& sourceMesh = nullptr);
        void drainMeshUpdateQueue();
        void drainRenderQueue();
        void drainMeshColorUpdates();
        void materializeMesh(const UpdateMeshParam& params,
                             const Common::SharedPtr<EgLab::ME::Mesh>& sourceMesh);
        void updateMeshHighlight();

    private:
        struct QueuedMeshUpdate
        {
            UpdateMeshParam params;
            Common::SharedPtr<ME::Mesh> sourceMesh;
        };

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

        CommandBus _renderUpdateBus{1};

        std::mutex _meshUpdateMutex;
        Common::DynamicArray<QueuedMeshUpdate> _meshUpdateQueue;
        std::mutex _renderUpdateMutex;
        Common::DynamicArray<RenderPacket> _renderUpdateQueue;

        std::mutex _meshColorMutex;
        Common::DynamicArray<MeshColor> _meshColors;
        Common::DynamicArray<Common::SharedPtr<ME::Mesh>> _dirtyMeshColors;
        Common::DynamicArray<RenderPacket> _activeMeshPackets;

        std::mutex _highlightMutex;
        Common::SharedPtr<EgLab::ME::Mesh> _requestedHighlightedMesh;
        Common::SharedPtr<EgLab::ME::Mesh> _activeHighlightedMesh;
        Common::SharedPtr<RE::Shader> _highlightShader;
        Common::SharedPtr<RE::RenderPrimitive> _highlightPrimitive;

        friend class RenderUpdate;

        EgLab::RE::DeltaTime lastFrame;
        EgLab::RE::DeltaTime deltaTime;

        friend class EgLab::Common::Singleton<RenderWork>;
    };
} // namespace EgLab::Platform
