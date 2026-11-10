#pragma once

#include "Command/CommandIds.hpp"
#include "Command/CommandParam.hpp"
#include "Common/HashMap.hpp"
#include "Common/Message.hpp"
#include "Common/Protocol.hpp"
#include "Common/Publisher.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Singleton.hpp"
#include "Common/Subscriber.hpp"

namespace EgLab::Platform
{
    class IPanel;
    class HEMeshPanel;
    class MessageLogPanel;
    class MessageToast;
} // namespace EgLab::Platform

namespace EgLab::ME
{
    class Mesh;
}

namespace EgLab::Platform
{
    class PanelManager
    {
    public:
        PanelManager();

        void loop();

        Common::Return showPanel(const Common::String&);
        Common::Return queueShowPanel(const Common::String&);
        Common::Return hidePanel(const Common::String&);
        void openHEMeshPanel(const Common::SharedPtr<ME::Mesh>& mesh,
                             const Common::String& meshName);

        // 接收来自 Common::MessageCenter 回调的消息并分发到两个消息面板。
        void handleMessage(const Common::Message& message);

        // const Common::SharedPtr<IPanel> getPanel() const;

    private:
        Common::HashMap<Common::String, Common::SharedPtr<IPanel>> _panels;
        Common::SharedPtr<HEMeshPanel> _heMeshPanel;
        Common::SharedPtr<MessageLogPanel> _messageLogPanel;
        Common::SharedPtr<MessageToast> _messageToast;
        Common::MemoryQueue _requestQueue;
        Common::Publisher _publisher;
        Common::Subscriber _subscriber;
    };
} // namespace EgLab::Platform