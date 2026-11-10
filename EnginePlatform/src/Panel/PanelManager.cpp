#include "Panel/PanelManager.hpp"

#include <algorithm>
#include <cstring>

#include "Common/MessageCenter.hpp"
#include "DataBase/Context.hpp"
#include "Panel/BackGroundSettingPanel.hpp"
#include "Panel/CameraSettingPanel.hpp"
#include "Panel/CommandParamWizardPanel.hpp"
#include "Panel/DataTreePanel.hpp"
#include "Panel/HEMeshDataPanel.hpp"
#include "Panel/HEMeshPanel.hpp"
#include "Panel/IPanel.hpp"
#include "Panel/LightSettingPanel.hpp"
#include "Panel/MeshImportPanel.hpp"
#include "Panel/MeshRenderSettingPanel.hpp"
#include "Panel/MessageLogPanel.hpp"
#include "Panel/MessageToast.hpp"
#include "Panel/ShortcutSettingPanel.hpp"

namespace
{
    // MessageCenter 的回调：EnginePlatform 将处理函数塞入回调，
    // 消息到达时转发给 PanelManager（UI 层）进行分发。
    void onMessageArrived(const EgLab::Common::Message& message)
    {
        EgLab::Platform::Context::instance().getPanelManager().handleMessage(message);
    }
} // namespace

namespace EgLab::Platform
{
    PanelManager::PanelManager()
        : _requestQueue("Panel Manager", true),
          _publisher(_requestQueue),
          _subscriber(_requestQueue)
    {
        _subscriber.subscrib(static_cast<uint32_t>(EventId::ShowPanel));
        _panels[MeshImportPanel().getPanelName()] = Common::makeShared<MeshImportPanel>();
        _panels[CameraSettingPanel().getPanelName()] = Common::makeShared<CameraSettingPanel>();
        _panels[BackGroundSettingPanel().getPanelName()] =
            Common::makeShared<BackGroundSettingPanel>();
        _panels[DataTreePanel().getPanelName()] = Common::makeShared<DataTreePanel>();
        _panels[HEMeshDataPanel().getPanelName()] = Common::makeShared<HEMeshDataPanel>();
        _heMeshPanel = Common::makeShared<HEMeshPanel>();
        _panels[_heMeshPanel->getPanelName()] = _heMeshPanel;
        _panels[MeshRenderSettingPanel().getPanelName()] =
            Common::makeShared<MeshRenderSettingPanel>();
        _panels[LightSettingPanel().getPanelName()] = Common::makeShared<LightSettingPanel>();
        _panels[ShortcutSettingPanel().getPanelName()] = Common::makeShared<ShortcutSettingPanel>();
        _panels[CommandParamWizardPanel().getPanelName()] =
            Common::makeShared<CommandParamWizardPanel>();

        _messageLogPanel = Common::makeShared<MessageLogPanel>();
        _panels[_messageLogPanel->getPanelName()] = _messageLogPanel;
        _messageToast = Common::makeShared<MessageToast>();

        // EnginePlatform 将消息处理函数塞入 Common 层回调。
        Common::MessageCenter::instance().setHandler(&onMessageArrived);
    }

    Common::Return PanelManager::showPanel(const Common::String& name)
    {
        auto it = _panels.find(name);
        if (it != _panels.end())
        {
            it.data().second->show();
            return Common::Return::Succeed;
        }
        return Common::Return::Failed;
    }

    Common::Return PanelManager::queueShowPanel(const Common::String& name)
    {
        ShowPanelParam param{};
        const size_t copySize = std::min(name.size(), sizeof(param.panelName) - 1);
        std::memcpy(param.panelName, name.c_str(), copySize);
        return _publisher.publish(static_cast<uint32_t>(EventId::ShowPanel), param)
                   ? Common::Return::Succeed
                   : Common::Return::Failed;
    }

    Common::Return PanelManager::hidePanel(const Common::String& name)
    {
        auto it = _panels.find(name);
        if (it != _panels.end())
        {
            it.data().second->close();
            return Common::Return::Succeed;
        }
        return Common::Return::Failed;
    }

    void PanelManager::openHEMeshPanel(const Common::SharedPtr<ME::Mesh>& mesh,
                                       const Common::String& meshName)
    {
        _heMeshPanel->setMesh(mesh, meshName);
        _heMeshPanel->show();
    }

    void PanelManager::loop()
    {
        uint32_t eventId = 0;
        ShowPanelParam showRequest{};
        while (_subscriber.tryReceive(eventId, showRequest))
        {
            showPanel(Common::String(showRequest.panelName));
        }

        // 取出消息中心回调中的消息并分发给消息面板。
        Common::MessageCenter::instance().poll();

        for (auto& [name, panel] : _panels)
        {
            if (panel)
            {
                if (panel->isShow())
                {
                    panel->render();
                }
            }
        }

        // 及时消息弹窗不放入 _panels，单独渲染。
        if (_messageToast && _messageToast->isShow())
        {
            _messageToast->render();
        }
    }

    void PanelManager::handleMessage(const Common::Message& message)
    {
        if (_messageLogPanel)
        {
            _messageLogPanel->pushMessage(message);
        }
        if (_messageToast)
        {
            _messageToast->pushMessage(message);
        }
    }

} // namespace EgLab::Platform