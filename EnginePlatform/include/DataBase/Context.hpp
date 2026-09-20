#pragma once

#include "Core/Application.hpp"
#include "Panel/PanelManager.hpp"

namespace EgLab::Platform
{
    class Application;
    class Context : public Common::Singleton<Context>
    {
    public:
        Context();

        ~Context();

        inline PanelManager& getPanelManager()
        {
            return _panelManager;
        }

        inline Application& getApplication()
        {
            return _application;
        }

    protected:
        friend class Common::Singleton<Context>;

    private:
        PanelManager _panelManager;

        Application _application;
    };
} // namespace EgLab::Platform