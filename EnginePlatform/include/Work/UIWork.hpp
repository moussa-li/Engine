#pragma once

#include <atomic>
#include <thread>

#include "Command/CommandIds.hpp"
#include "Command/CommandManager.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Singleton.hpp"

namespace EgLab::RE
{
    class Window;
}

namespace EgLab::Platform
{
    class UIWork : public EgLab::Common::Singleton<UIWork>
    {
    public:
        UIWork();
        ~UIWork();

        void start();
        void stop();
        void bindWindow(const Common::SharedPtr<EgLab::RE::Window>& window);

        // UIWork is a dedicated UI event and draw lane.
        // It should insulate ImGui/Panel orchestration from the command worker pool.
        void subscribe(EventId eventId);
        void onUIEvent(const EventPacket& packet);
        void renderFrame();

        void initImGui();
        void shutdownImGui();

        void activeContext() const;
        void deactiveContext() const;

        void beginImGuiFrame();
        void renderImGuiFrame();

        // Rubber-band selection tool driven by the ribbon toolbar. The tool draws its
        // rectangle in the ImGui background list and masks camera input while active.
        bool isBoxSelectionEnabled() const
        {
            return _boxSelectionEnabled;
        }
        void setBoxSelectionEnabled(bool enabled);
        void requestClearSelection();

    private:
        void renderBoxSelectionTool();

    private:
        std::thread _thread;
        std::atomic<bool> _running{false};
        Common::SharedPtr<EgLab::RE::Window> _window;
        bool _boxSelectionEnabled{false};
        bool _selectionDragging{false};
        float _selectionStartX{0.0f};
        float _selectionStartY{0.0f};
        float _selectionCurrentX{0.0f};
        float _selectionCurrentY{0.0f};

        friend class EgLab::Common::Singleton<UIWork>;
    };
} // namespace EgLab::Platform
