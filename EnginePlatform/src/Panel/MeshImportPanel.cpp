#include "Panel/MeshImportPanel.hpp"

#include "Command/CommandManager.hpp"
#include "ImGuiFileDialog.h"
#include "RenderEngine/Core/ShaderLib.hpp"

namespace EgLab::Platform
{
    MeshImportPanel::MeshImportPanel()
    {
    }

    void MeshImportPanel::abortShow()
    {
        IGFD::FileDialogConfig config;
        config.path = ".";
        ImGuiFileDialog::Instance()->OpenDialog("ImportMesh", "Please Select a Mesh File", ".msh",
                                                config);
    }

    void MeshImportPanel::render()
    {
        if (ImGuiFileDialog::Instance()->Display("ImportMesh"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                EventPacket packet;
                packet.setData("fileDir", Common::String(filePath.c_str()));
                CommandManager::instance().call(EventId::MeshImport, Common::move(packet));
            }
            ImGuiFileDialog::Instance()->Close();

            // this->hide();
        }
    }

} // namespace EgLab::Platform