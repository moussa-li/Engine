#pragma once

#include <atomic>
#include <mutex>
#include <thread>

#include "Command/CommandIds.hpp"
#include "Command/CommandParam.hpp"
#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Singleton.hpp"
#include "RenderEngine/Core/Definites.hpp"

namespace EgLab::RE
{
    class Window;
    class Renderer;
    class Scene;
    class Camera;
} // namespace EgLab::RE

namespace EgLab::Platform
{
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

        // RenderWork is a dedicated render event listener.
        // Only EventId::UpdateMesh is expected to enter this lane.
        void subscribe(EventId eventId);
        void onUpdateMesh(const UpdateMeshParam& params);

    private:
        void queueMeshUpdate(const UpdateMeshParam& params);
        void drainMeshUpdateQueue();
        void materializeMesh(const UpdateMeshParam& params);

    private:
        std::thread _thread;
        std::atomic<bool> _running{false};
        Common::SharedPtr<EgLab::RE::Window> _window;
        Common::SharedPtr<EgLab::RE::Renderer> _renderer;
        Common::SharedPtr<EgLab::RE::Scene> _scene;
        Common::SharedPtr<EgLab::RE::Camera> _camera;

        std::mutex _meshUpdateMutex;
        Common::DynamicArray<UpdateMeshParam> _meshUpdateQueue;

        EgLab::RE::DeltaTime lastFrame;
        EgLab::RE::DeltaTime deltaTime;

        friend class EgLab::Common::Singleton<RenderWork>;
    };
} // namespace EgLab::Platform
