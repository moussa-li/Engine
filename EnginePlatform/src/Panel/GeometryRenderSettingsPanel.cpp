#include "Panel/GeometryRenderSettingsPanel.hpp"

#include "Command/CommandManager.hpp"
#include "Core/IniManager.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    namespace
    {
        constexpr const char* GEOMETRY_DISPLAY_MODE_KEY = "geometry.displayMode";
        constexpr const char* GEOMETRY_SHOW_VERTICES_KEY = "geometry.showVertices";
        constexpr const char* GEOMETRY_ZEBRA_KEY = "geometry.zebra";
        constexpr const char* GEOMETRY_ZEBRA_FREQUENCY_KEY = "geometry.zebraFrequency";
        constexpr const char* GEOMETRY_ZEBRA_LIGHT_X_KEY = "geometry.zebraLightX";
        constexpr const char* GEOMETRY_ZEBRA_LIGHT_Y_KEY = "geometry.zebraLightY";
        constexpr const char* GEOMETRY_ZEBRA_LIGHT_Z_KEY = "geometry.zebraLightZ";
        constexpr const char* GEOMETRY_UV_KEY = "geometry.uv";
        constexpr const char* GEOMETRY_TESS_LINEAR_KEY = "geometry.tessellation.linearDeflection";
        constexpr const char* GEOMETRY_TESS_ANGULAR_KEY = "geometry.tessellation.angularDeflection";
        constexpr const char* GEOMETRY_TESS_RELATIVE_KEY = "geometry.tessellation.relative";
        constexpr const char* GEOMETRY_TESS_PARALLEL_KEY = "geometry.tessellation.parallel";
        constexpr float GEOMETRY_ZEBRA_DEFAULT_LIGHT_X = 0.0f;
        constexpr float GEOMETRY_ZEBRA_DEFAULT_LIGHT_Y = 0.0f;
        constexpr float GEOMETRY_ZEBRA_DEFAULT_LIGHT_Z = 1.0f;
    } // namespace

    GeometryRenderSettingsPanel::GeometryRenderSettingsPanel()
    {
        auto& ini = IniManager::instance();
        const int savedMode = ini.getInt(GEOMETRY_DISPLAY_MODE_KEY, 0);
        if (GeometryDisplayModeInfo::isValid(static_cast<uint32_t>(savedMode)))
        {
            _mode = static_cast<GeometryDisplayMode>(savedMode);
        }
        _showVertices = ini.getInt(GEOMETRY_SHOW_VERTICES_KEY, 1) != 0;

        _zebra.enabled = ini.getBool(GEOMETRY_ZEBRA_KEY, false);
        _zebra.frequency = GeometryZebra::clampFrequency(
            ini.getFloat(GEOMETRY_ZEBRA_FREQUENCY_KEY, GeometryZebra::DEFAULT_FREQUENCY));
        _zebra.lightDirection = GeometryZebra::clampDirection(
            Common::Vector3f(ini.getFloat(GEOMETRY_ZEBRA_LIGHT_X_KEY, GEOMETRY_ZEBRA_DEFAULT_LIGHT_X),
                             ini.getFloat(GEOMETRY_ZEBRA_LIGHT_Y_KEY, GEOMETRY_ZEBRA_DEFAULT_LIGHT_Y),
                             ini.getFloat(GEOMETRY_ZEBRA_LIGHT_Z_KEY, GEOMETRY_ZEBRA_DEFAULT_LIGHT_Z)));

        _uvEnabled = ini.getBool(GEOMETRY_UV_KEY, false);

        _tessLinearDeflection = ini.getFloat(GEOMETRY_TESS_LINEAR_KEY, 0.1f);
        _tessAngularDeflection = ini.getFloat(GEOMETRY_TESS_ANGULAR_KEY, 0.5f);
        _tessRelative = ini.getBool(GEOMETRY_TESS_RELATIVE_KEY, false);
        _tessParallel = ini.getBool(GEOMETRY_TESS_PARALLEL_KEY, true);
    }

    void GeometryRenderSettingsPanel::applyVertexDisplay(bool visible)
    {
        _showVertices = visible;
        IniManager::instance().setInt(GEOMETRY_SHOW_VERTICES_KEY, visible ? 1 : 0);

        EventPacket packet;
        packet.setData("visible", static_cast<uint32_t>(visible ? 1 : 0));
        CommandManager::instance().call(EventId::SetGeometryVertexDisplay, Common::move(packet));
    }

    void GeometryRenderSettingsPanel::configureWindow() const
    {
        // The content grew with the zebra controls, so keep an older saved window size tall
        // enough to show them while still allowing the user to resize the panel.
        ImGui::SetNextWindowSizeConstraints(ImVec2(320.0f, 560.0f), ImVec2(900.0f, 900.0f));
        ImGui::SetNextWindowSize(ImVec2(360, 600), ImGuiCond_FirstUseEver);
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

    void GeometryRenderSettingsPanel::applyZebra(bool enabled, float frequency,
                                                 const Common::Vector3f& lightDirection)
    {
        _zebra.enabled = enabled;
        _zebra.frequency = GeometryZebra::clampFrequency(frequency);
        // Keep the raw UI vector so the sliders stay responsive; the engine normalises it and
        // falls back to the default when it is degenerate.
        _zebra.lightDirection = lightDirection;
        IniManager::instance().setInt(GEOMETRY_ZEBRA_KEY, _zebra.enabled ? 1 : 0);
        IniManager::instance().setFloat(GEOMETRY_ZEBRA_FREQUENCY_KEY, _zebra.frequency);
        IniManager::instance().setFloat(GEOMETRY_ZEBRA_LIGHT_X_KEY, _zebra.lightDirection.x());
        IniManager::instance().setFloat(GEOMETRY_ZEBRA_LIGHT_Y_KEY, _zebra.lightDirection.y());
        IniManager::instance().setFloat(GEOMETRY_ZEBRA_LIGHT_Z_KEY, _zebra.lightDirection.z());

        EventPacket packet;
        packet.setData("enabled", static_cast<uint32_t>(_zebra.enabled ? 1 : 0));
        packet.setData("frequency", _zebra.frequency);
        packet.setData("lightX", _zebra.lightDirection.x());
        packet.setData("lightY", _zebra.lightDirection.y());
        packet.setData("lightZ", _zebra.lightDirection.z());
        CommandManager::instance().call(EventId::SetGeometryZebra, Common::move(packet));
    }

    void GeometryRenderSettingsPanel::applyUv(bool enabled)
    {
        _uvEnabled = enabled;
        IniManager::instance().setInt(GEOMETRY_UV_KEY, _uvEnabled ? 1 : 0);

        EventPacket packet;
        packet.setData("enabled", static_cast<uint32_t>(_uvEnabled ? 1 : 0));
        CommandManager::instance().call(EventId::SetGeometryUv, Common::move(packet));
    }

    void GeometryRenderSettingsPanel::applyTessellation()
    {
        auto& ini = IniManager::instance();
        ini.setFloat(GEOMETRY_TESS_LINEAR_KEY, _tessLinearDeflection);
        ini.setFloat(GEOMETRY_TESS_ANGULAR_KEY, _tessAngularDeflection);
        ini.setInt(GEOMETRY_TESS_RELATIVE_KEY, _tessRelative ? 1 : 0);
        ini.setInt(GEOMETRY_TESS_PARALLEL_KEY, _tessParallel ? 1 : 0);

        EventPacket packet;
        packet.setData("linearDeflection", _tessLinearDeflection);
        packet.setData("angularDeflection", _tessAngularDeflection);
        packet.setData("relative", static_cast<uint32_t>(_tessRelative ? 1 : 0));
        packet.setData("parallel", static_cast<uint32_t>(_tessParallel ? 1 : 0));
        CommandManager::instance().call(EventId::SetGeometryTessellation, Common::move(packet));
        _tessStatus = "Re-tessellation requested";
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
        if (ImGui::Checkbox("Show vertices", &_showVertices))
        {
            applyVertexDisplay(_showVertices);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Only the nodes that reference a B-Rep vertex are drawn; this is "
                              "independent of the mesh node rendering setting.");
        }
        ImGui::TextDisabled("Geometry models never draw their tessellation nodes.");

        ImGui::Separator();
        bool zebraEnabled = _zebra.enabled;
        if (ImGui::Checkbox("Zebra stripes", &zebraEnabled))
        {
            applyZebra(zebraEnabled, _zebra.frequency, _zebra.lightDirection);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Isophote stripe analysis on geometry faces: the bands bend where "
                              "the surface curvature changes.");
        }

        float zebraFrequency = _zebra.frequency;
        ImGui::BeginDisabled(!_zebra.enabled);
        if (ImGui::SliderFloat("Stripe density", &zebraFrequency, GeometryZebra::MIN_FREQUENCY,
                               GeometryZebra::MAX_FREQUENCY, "%.1f"))
        {
            applyZebra(_zebra.enabled, zebraFrequency, _zebra.lightDirection);
        }

        float zebraLight[3] = {_zebra.lightDirection.x(), _zebra.lightDirection.y(),
                               _zebra.lightDirection.z()};
        if (ImGui::SliderFloat3("Stripe light", zebraLight, -1.0f, 1.0f, "%.2f"))
        {
            applyZebra(_zebra.enabled, _zebra.frequency,
                       Common::Vector3f(zebraLight[0], zebraLight[1], zebraLight[2]));
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Virtual light direction the stripes mirror. It is owned by the "
                              "zebra shader, so the global lighting and the camera do not move "
                              "the pattern.");
        }
        if (ImGui::Button("Reset stripe light"))
        {
            applyZebra(_zebra.enabled, _zebra.frequency,
                       Common::Vector3f(GEOMETRY_ZEBRA_DEFAULT_LIGHT_X,
                                        GEOMETRY_ZEBRA_DEFAULT_LIGHT_Y,
                                        GEOMETRY_ZEBRA_DEFAULT_LIGHT_Z));
        }
        ImGui::EndDisabled();
        ImGui::TextWrapped("Stripes replace the shaded color of the faces; edges and picking "
                           "are unaffected.");

        ImGui::Separator();
        if (ImGui::Checkbox("Surface UV", &_uvEnabled))
        {
            applyUv(_uvEnabled);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Colours geometry faces from the surface UV taken at import and "
                              "interpolated across each triangle. Takes precedence over zebra.");
        }
        ImGui::TextDisabled("UV comes from each B-Rep face's surface parameters (normalised per "
                            "face).");

        ImGui::Separator();
        ImGui::TextUnformatted("Tessellation (render mesh)");
        ImGui::TextDisabled("Applies to new imports and re-tessellates existing geometry models.");
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::SliderFloat("Linear deflection", &_tessLinearDeflection, 0.001f, 5.0f, "%.3f",
                           ImGuiSliderFlags_Logarithmic);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Maximum chordal deviation between the tessellation and the exact "
                              "surface. Smaller values give a denser, more accurate mesh.");
        }
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::SliderFloat("Angular deflection", &_tessAngularDeflection, 0.05f, 1.5f, "%.2f rad",
                           ImGuiSliderFlags_Logarithmic);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Maximum angle between adjacent tessellation segments. Smaller "
                              "values refine curved regions.");
        }
        ImGui::Checkbox("Relative deflection", &_tessRelative);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Scale the linear deflection by the model bounding box instead of "
                              "using model units.");
        }
        ImGui::Checkbox("Parallel meshing", &_tessParallel);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Tessellate faces in parallel. Disable for deterministic, "
                              "single-threaded output.");
        }
        if (ImGui::Button("Apply tessellation"))
        {
            applyTessellation();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset tessellation"))
        {
            _tessLinearDeflection = 0.1f;
            _tessAngularDeflection = 0.5f;
            _tessRelative = false;
            _tessParallel = true;
            applyTessellation();
        }
        if (_tessStatus.size() > 0)
        {
            ImGui::TextDisabled("%s", _tessStatus.c_str());
        }

        ImGui::Separator();
        ImGui::Text("Current mode: %s",
                    GeometryDisplayModeInfo::toDisplayName(_mode).c_str());
    }
} // namespace EgLab::Platform
