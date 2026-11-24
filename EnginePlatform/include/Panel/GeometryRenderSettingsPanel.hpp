#pragma once

#include <cstdint>

#include "Panel/IPanel.hpp"
#include "Work/GeometryDisplayMode.hpp"

namespace EgLab::Platform
{
    /// Geometry render settings: selects how imported B-Rep models are drawn.
    ///
    /// Mode changes are dispatched through CommandManager so the UI never mutates the render
    /// state directly and AI clients can drive the same behaviour.
    class GeometryRenderSettingsPanel : public IPanel
    {
    public:
        GeometryRenderSettingsPanel();

        Common::String getPanelName() override
        {
            return Common::String("GeometryRenderSettings");
        }

    protected:
        const char* getWindowTitle() const override
        {
            return "Geometry Render Settings";
        }

        void configureWindow() const override;
        void renderImpl() override;

    private:
        void applyMode(GeometryDisplayMode mode);

        GeometryDisplayMode _mode{GeometryDisplayMode::Mesh};
        Common::String _status;
    };
} // namespace EgLab::Platform
