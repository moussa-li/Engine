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
    void renderRibbonToolbar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                { /* ... */
                    Context::instance().getPanelManager().showPanel("MeshImport");
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
                    Context::instance().getPanelManager().showPanel("CameraSetting");
                }

                if (ImGui::MenuItem("background setting", "f5"))
                {
                    Context::instance().getPanelManager().showPanel("BackGroundSetting");
                }
                if (ImGui::MenuItem("Light Settings"))
                {
                    Context::instance().getPanelManager().showPanel("LightSetting");
                }
                if (ImGui::MenuItem("Data Tree"))
                {
                    Context::instance().getPanelManager().showPanel("DataTree");
                }
                if (ImGui::MenuItem("Mesh Render Settings"))
                {
                    Context::instance().getPanelManager().showPanel("MeshRenderSettings");
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

} // namespace EgLab::Platform