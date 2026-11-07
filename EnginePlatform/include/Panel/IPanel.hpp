#pragma once

#include "Common/String.hpp"

namespace EgLab::Platform
{
    class IPanel
    {
    public:
        IPanel();
        virtual ~IPanel();

        virtual Common::String getPanelName() = 0;

        inline bool isShow() const
        {
            return _show;
        }

        void show();
        void close();

        // Non-virtual interface: owns the ImGui window frame (Begin/End),
        // applies shared behavior (close on Escape) and delegates the content
        // to renderImpl().
        void render();

    protected:
        // Panel content drawn inside the window opened by render().
        virtual void renderImpl() = 0;

        // ImGui window title used by render().
        virtual const char* getWindowTitle() const = 0;

        // Whether render() should open an ImGui window for this panel.
        virtual bool usesWindowFrame() const
        {
            return true;
        }

        // Called before ImGui::Begin() to configure the window (e.g. default size).
        virtual void configureWindow() const
        {
        }

        // Called after ImGui::End() while the panel is shown.
        virtual void afterRenderFrame()
        {
        }

        virtual void abortShow() {};
        virtual void abortClose() {};

    private:
        void closeOnEscape();

    protected:
        bool _show{false};
    };
} // namespace EgLab::Platform