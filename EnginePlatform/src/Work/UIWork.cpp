#include "Work/UIWork.hpp"

#include <GL/glew.h>

#include <chrono>

#include "Common/Log.hpp"
#include "Core/MainMenuBar.hpp"
#include "DataBase/Context.hpp"
#include "GLFW/glfw3.h"
#include "Panel/PanelManager.hpp"
#include "RenderEngine/Core/Window.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace EgLab::Platform
{
    UIWork::UIWork()
    {
    }

    UIWork::~UIWork()
    {
        stop();
    }

    void UIWork::bindWindow(const Common::SharedPtr<EgLab::RE::Window>& window)
    {
        _window = window;
    }

    void UIWork::start()
    {
        if (_running.load())
        {
            return;
        }

        _running = true;

        initImGui();
    }

    void UIWork::stop()
    {
        if (_running == false) return;
        _running = false;
        shutdownImGui();
    }

    void UIWork::subscribe(EventId eventId)
    {
        (void)eventId;
    }

    void UIWork::onUIEvent(const EventPacket& packet)
    {
        (void)packet;
        LOG(INFO) << "UIWork::onUIEvent() receives UI packet and routes to panel/imGui";
    }

    void UIWork::renderFrame()
    {
        if (_window)
        {
            activeContext();
            glfwPollEvents();
            beginImGuiFrame();
            Context::instance().getPanelManager().loop();
            _window->maskEvent(ImGui::GetIO().WantCaptureMouse);
            renderImGuiFrame();
            deactiveContext();
        }
    }

    void UIWork::activeContext() const
    {
        if (_window != nullptr)
        {
            _window->activeContext();
        }
    }

    void UIWork::deactiveContext() const
    {
        if (_window != nullptr)
        {
            _window->deactiveContext();
        }
    }

    void UIWork::initImGui()
    {
        activeContext();
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();

        style.WindowRounding = 6.0f;
        style.FrameRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 4.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 4.0f;

        style.WindowPadding = ImVec2(10, 10);
        style.FramePadding = ImVec2(8, 5);
        style.ItemSpacing = ImVec2(8, 6);
        style.ItemInnerSpacing = ImVec2(6, 6);

        ImGui_ImplGlfw_InitForOpenGL(_window->getNative(), true);
        ImGui_ImplOpenGL3_Init("#version 330");
        deactiveContext();
    }

    void UIWork::beginImGuiFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void UIWork::renderImGuiFrame()
    {
        renderRibbonToolbar();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void UIWork::shutdownImGui()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
} // namespace EgLab::Platform
