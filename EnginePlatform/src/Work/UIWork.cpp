#include "Work/UIWork.hpp"

#include <GL/glew.h>

#include <chrono>

#include "Common/Log.hpp"
#include "Core/MainMenuBar.hpp"
#include "GLFW/glfw3.h"
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
