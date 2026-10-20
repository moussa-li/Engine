#include "Panel/PanelManager.hpp"

#include <algorithm>
#include <cstring>

#include "Panel/BackGroundSettingPanel.hpp"
#include "Panel/CameraSettingPanel.hpp"
#include "Panel/DataTreePanel.hpp"
#include "Panel/IPanel.hpp"
#include "Panel/LightSettingPanel.hpp"
#include "Panel/MeshImportPanel.hpp"
#include "Panel/MeshRenderSettingPanel.hpp"

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
        _panels[MeshRenderSettingPanel().getPanelName()] =
            Common::makeShared<MeshRenderSettingPanel>();
        _panels[LightSettingPanel().getPanelName()] = Common::makeShared<LightSettingPanel>();
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

    void PanelManager::loop()
    {
        uint32_t eventId = 0;
        ShowPanelParam showRequest{};
        while (_subscriber.tryReceive(eventId, showRequest))
        {
            showPanel(Common::String(showRequest.panelName));
        }

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
    }

} // namespace EgLab::Platform