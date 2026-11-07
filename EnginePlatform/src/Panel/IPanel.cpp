#include "Panel/IPanel.hpp"

#include "imgui.h"

namespace EgLab::Platform
{
    IPanel::IPanel()
    {
    }

    IPanel::~IPanel()
    {
    }

    void IPanel::show()
    {
        _show = true;
        abortShow();
    }

    void IPanel::close()
    {
        _show = false;
        abortClose();
    }

    void IPanel::render()
    {
        if (!usesWindowFrame())
        {
            closeOnEscape();
            if (_show) renderImpl();
            return;
        }

        configureWindow();
        if (ImGui::Begin(getWindowTitle(), &_show))
        {
            closeOnEscape();
            renderImpl();
        }
        ImGui::End();
        afterRenderFrame();
    }

    void IPanel::closeOnEscape()
    {
        if (!ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            return;
        }

        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) ||
            ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows))
        {
            close();
        }
    }
} // namespace EgLab::Platform