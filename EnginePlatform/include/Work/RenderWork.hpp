#pragma once

#include <atomic>
#include <thread>

#include "Command/CommandIds.hpp"
#include "Command/CommandParam.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Singleton.hpp"

namespace EgLab::RE
{
    class Window;
}

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

        // RenderWork is a dedicated render event listener.
        // Only EventId::UpdateMesh is expected to enter this lane.
        void subscribe(EventId eventId);
        void onUpdateMesh(const UpdateMeshParam& params);

    private:
        std::thread _thread;
        std::atomic<bool> _running{false};
        Common::SharedPtr<EgLab::RE::Window> _window;

        friend class EgLab::Common::Singleton<RenderWork>;
    };
} // namespace EgLab::Platform
