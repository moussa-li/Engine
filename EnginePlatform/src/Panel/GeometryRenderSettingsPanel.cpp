#include "Panel/GeometryRenderSettingsPanel.hpp"

#include "Command/CommandManager.hpp"
#include "Core/IniManager.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    namespace
    {
        constexpr const char* GEOMETRY_DISPLAY_MODE_KEY = "geometry.displayMode";
    } // namespace

    GeometryRenderSettingsPanel::GeometryRenderSettingsPanel()
    {
        auto& ini = IniManager::instance();
        const int savedMode = ini.getInt(GEOMETRY_DISPLAY_MODE_KEY, 0);
        if (GeometryDisplayModeInfo::isValid(static_cast<uint32_t>(savedMode)))
        {
            _mode = static_cast<GeometryDisplayMode>(savedMode);
        }
    }

    void GeometryRenderSettingsPanel::configureWindow() const
    {
        ImGui::SetNextWindowSize(ImVec2(360, 200), ImGuiCond_FirstUseEver);
    }

    void GeometryRenderSettingsPanel::applyMode(GeometryDisplayMode mode)
    {
        _mode = mode;
        IniManager::instance().setInt(GEOMETRY_DISPLAY_MODE_KEY,
                                      static_cast<int>(mode));

        EventPacket packet;
        packet.setData("mode", static_cast<uint32_t>(mode));
        CommandManager::instance().call(EventId::SetGeometryRenderMode, Common::move(packet));

        _status = GeometryDisplayModeInfo::toDisplayName(mode);
    }

    void GeometryRenderSettingsPanel::renderImpl()
    {
        ImGui::TextWrapped(
            "Display mode for imported B-Rep geometry models (STEP/BREP). Regular meshes are "
            "not affected.");
        ImGui::Separator();

        const struct
        {
            GeometryDisplayMode mode;
            const char* label;
            const char* hint;
        } options[] = {
            {GeometryDisplayMode::Mesh, "1. Mesh",
             "Faces shaded with the dense triangle wireframe (current mesh look)."},
            {GeometryDisplayMode::FaceOutline, "2. Face outline",
             "Faces shaded, but only each face's outer frame is drawn."},
            {GeometryDisplayMode::EdgeOnly, "3. Edges only",
             "Faces are not rendered; only the geometry lines are drawn."},
        };

        for (const auto& option : options)
        {
            const bool selected = _mode == option.mode;
            if (ImGui::RadioButton(option.label, selected))
            {
                applyMode(option.mode);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", option.hint);
            }
            ImGui::TextDisabled("%s", option.hint);
            ImGui::Spacing();
        }

        ImGui::Separator();
        ImGui::Text("Current mode: %s",
                    GeometryDisplayModeInfo::toDisplayName(_mode).c_str());
    }
} // namespace EgLab::Platform
