#pragma once

#include "Command/CommandIds.hpp"
#include "Command/CommandParam.hpp"
#include "Common/HashMap.hpp"
#include "Common/Protocol.hpp"
#include "Common/Publisher.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Singleton.hpp"
#include "Common/Subscriber.hpp"

namespace EgLab::Platform
{
    class IPanel;
    class PanelManager
    {
    public:
        PanelManager();

        void loop();

        Common::Return showPanel(const Common::String&);
        Common::Return queueShowPanel(const Common::String&);
        Common::Return hidePanel(const Common::String&);

        // const Common::SharedPtr<IPanel> getPanel() const;

    private:
        Common::HashMap<Common::String, Common::SharedPtr<IPanel>> _panels;
        Common::MemoryQueue _requestQueue;
        Common::Publisher _publisher;
        Common::Subscriber _subscriber;
    };
} // namespace EgLab::Platform