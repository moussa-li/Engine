#include "Core/MainMenuBar.hpp"

#include <filesystem>

#include "Command/CommandManager.hpp"
#include "DataBase/Context.hpp"
#include "ImGuiFileDialog.h"

namespace EgLab
{
    inline Common::String getTestDataDir(const Common::String& file)
    {
        std::filesystem::path root(__FILE__);
        root = root.parent_path().parent_path().parent_path();
        root /= "test/testdata";
        root /= file.c_str();
        return Common::String(root.generic_string().c_str());
    }
} // namespace EgLab

namespace EgLab::Platform
{
    void requestShowPanel(const Common::String& panelName)
    {
        EventPacket packet;
        packet.setData("panelName", panelName);
        CommandManager::instance().call(EventId::ShowPanel, Common::move(packet));
    }

    void renderRibbonToolbar()
    {
        const bool openRequested =
            ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_RouteGlobal);
        if (openRequested)
        {
            requestShowPanel("MeshImport");
        }

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                { /* ... */
                    requestShowPanel("MeshImport");
                }

                if (ImGui::MenuItem("Save", "Ctrl+S"))
                { /* ... */
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit"))
                { /* ... */
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("camera setting", "f5"))
                {
                    requestShowPanel("CameraSetting");
                }

                if (ImGui::MenuItem("background setting", "f5"))
                {
                    requestShowPanel("BackGroundSetting");
                }
                if (ImGui::MenuItem("Light Settings"))
                {
                    requestShowPanel("LightSetting");
                }
                if (ImGui::MenuItem("Data Tree"))
                {
                    requestShowPanel("DataTree");
                }
                if (ImGui::MenuItem("Mesh Render Settings"))
                {
                    requestShowPanel("MeshRenderSettings");
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Mesh"))
            {
                if (ImGui::MenuItem("Half Edge Mesh"))
                {
                    requestShowPanel("HEMesh");
                }
            }

            ImGui::EndMainMenuBar();
        }
    }

} // namespace EgLab::Platform