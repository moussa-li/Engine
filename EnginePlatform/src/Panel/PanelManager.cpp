#include "Panel/PanelManager.hpp"

#include "Panel/IPanel.hpp"
#include "Panel/MeshImportPanel.hpp"

namespace EgLab::Platform
{
    PanelManager::PanelManager()
    {
        _panels[MeshImportPanel().getPanelName()] = Common::makeShared<MeshImportPanel>();
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