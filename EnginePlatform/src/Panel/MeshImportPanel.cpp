#include "Panel/MeshImportPanel.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>

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
        ImGuiFileDialog::Instance()->OpenDialog(
            "ImportMesh", "Please Select a Mesh or Model File",
            "Mesh files{.msh,.grd,.stp,.step,.brp,.brep}", config);
    }

    void MeshImportPanel::renderImpl()
    {
        if (ImGuiFileDialog::Instance()->Display("ImportMesh"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();

                std::filesystem::path path(filePath);
                std::string extension = path.extension().string();
                std::transform(extension.begin(), extension.end(), extension.begin(),
                               [](unsigned char c) {
                                   return static_cast<char>(std::tolower(c));
                               });

                EventPacket packet;
                if (extension == ".stp" || extension == ".step" || extension == ".brp" ||
                    extension == ".brep")
                {
                    packet.setData("filePath", Common::String(filePath.c_str()));
                    CommandManager::instance().call(EventId::GeometryImport,
                                                    Common::move(packet));
                }
                else
                {
                    packet.setData("fileDir", Common::String(filePath.c_str()));
                    CommandManager::instance().call(EventId::MeshImport, Common::move(packet));
                }
            }
            ImGuiFileDialog::Instance()->Close();

            // this->hide();
        }
    }

} // namespace EgLab::Platform