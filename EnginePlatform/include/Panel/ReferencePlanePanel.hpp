#pragma once

#include "Panel/IPanel.hpp"
#include "RenderEngine/Core/ReferencePlane.hpp"

namespace EgLab::Platform
{
    /// Configures the three orthogonal reference planes drawn by the renderer.
    class ReferencePlanePanel : public IPanel
    {
    public:
        ReferencePlanePanel();
        ~ReferencePlanePanel() override;

        Common::String getPanelName() override
        {
            return "ReferencePlane";
        }

    protected:
        const char* getWindowTitle() const override
        {
            return "Reference Plane";
        }

        void configureWindow() const override;
        void renderImpl() override;
        void abortShow() override;

    private:
        static Common::String formatColor(const Common::Vector4f& color);

    private:
        RE::ReferencePlaneSettings _settings;
        bool _synced{false};
    };
} // namespace EgLab::Platform
