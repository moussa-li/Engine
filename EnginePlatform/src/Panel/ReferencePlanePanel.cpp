#include "Panel/ReferencePlanePanel.hpp"

#include <cstdio>

#include "Command/CommandManager.hpp"
#include "Common/Utils.hpp"
#include "DataBase/Context.hpp"
#include "RenderEngine/Core/Renderer.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    ReferencePlanePanel::ReferencePlanePanel()
    {
    }

    ReferencePlanePanel::~ReferencePlanePanel()
    {
    }

    void ReferencePlanePanel::configureWindow() const
    {
        ImGui::SetNextWindowSize(ImVec2(340, 300), ImGuiCond_FirstUseEver);
    }

    void ReferencePlanePanel::abortShow()
    {
        _synced = false;
    }

    Common::String ReferencePlanePanel::formatColor(const Common::Vector4f& color)
    {
        char buffer[96];
        std::snprintf(buffer, sizeof(buffer), "%.4f,%.4f,%.4f,%.4f", color[0], color[1],
                      color[2], color[3]);
        return Common::String(buffer);
    }

    void ReferencePlanePanel::renderImpl()
    {
        auto renderer = Context::instance().getApplication().renderer;
        if (!renderer)
        {
            return;
        }

        if (!_synced)
        {
            _settings = renderer->getReferencePlane().getSettings();
            _synced = true;
        }

        bool changed = false;

        changed |= ImGui::Checkbox("Enable Reference Plane", &_settings.enabled);
        changed |= ImGui::Checkbox("Show Grid", &_settings.showGrid);
        changed |= ImGui::SliderFloat("Bounds Scale", &_settings.scale, 0.1f, 5.0f, "%.2f");

        ImGui::Separator();
        ImGui::Text("Placement");
        int placement = static_cast<int>(_settings.placement);
        if (ImGui::RadioButton("BBox Center", &placement, 0))
        {
            changed = true;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("BBox Back Faces", &placement, 1))
        {
            changed = true;
        }
        _settings.placement = static_cast<RE::ReferencePlanePlacement>(placement);

        ImGui::Separator();
        ImGui::Text("Plane Colors");

        static const char* planeLabels[3] = {"YZ Plane (X normal)", "XZ Plane (Y normal)",
                                             "XY Plane (Z normal)"};
        for (int plane = 0; plane < 3; ++plane)
        {
            float color[4] = {_settings.colors[plane][0], _settings.colors[plane][1],
                              _settings.colors[plane][2], _settings.colors[plane][3]};
            if (ImGui::ColorEdit4(planeLabels[plane], color,
                                  ImGuiColorEditFlags_AlphaBar |
                                      ImGuiColorEditFlags_AlphaPreviewHalf))
            {
                _settings.colors[plane] =
                    Common::Vector4f(color[0], color[1], color[2], color[3]);
                changed = true;
            }
        }

        if (!changed)
        {
            return;
        }

        // Every setting reaches the renderer through the command layer so UI, cross-process
        // callers and the AI share the same entry point.
        EventPacket packet;
        packet.setData("enabled", _settings.enabled);
        packet.setData("showGrid", _settings.showGrid);
        packet.setData("placement", static_cast<uint32_t>(_settings.placement));
        packet.setData("scale", _settings.scale);
        packet.setData("colorX", formatColor(_settings.colors[0]));
        packet.setData("colorY", formatColor(_settings.colors[1]));
        packet.setData("colorZ", formatColor(_settings.colors[2]));
        CommandManager::instance().call(EventId::SetReferencePlane, Common::move(packet));
    }
} // namespace EgLab::Platform
