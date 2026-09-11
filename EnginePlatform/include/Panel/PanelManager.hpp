#pragma once

#include "Common/HashMap.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Singleton.hpp"
#include "Common/UniquePtr.hpp"

namespace EgLab::Platform
{
    class IPanel;
    class PanelManager
    {
    public:
        PanelManager();

        void loop();

        Common::Return addPanel(Common::SharedPtr<IPanel>);

        Common::Return showPanel(Common::String&);
        Common::Return hidePanel(Common::String&);

        const Common::SharedPtr<IPanel> getPanel() const;

    private:
        Common::HashMap<Common::String, Common::SharedPtr<IPanel>> _panels;
    };
} // namespace EgLab::Platform