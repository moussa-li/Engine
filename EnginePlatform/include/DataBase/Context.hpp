#pragma once

#include "Panel/PanelManager.hpp"

namespace EgLab::Platform
{
    class Context : public Common::Singleton<Context>
    {
    public:
        Context();

        ~Context();

        inline PanelManager& getPanelManager()
        {
            return _panelManager;
        }

    protected:
        friend class Common::Singleton<Context>;

    private:
        PanelManager _panelManager;
    };
} // namespace EgLab::Platform