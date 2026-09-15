#include "Core/MainMenuBar.hpp"

#include <filesystem>

#include "Command/CommandManager.hpp"
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
        bool showWireframe = false;
        bool showGrid = true;

        if (ImGuiFileDialog::Instance()->Display("文件选择"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                EventPacket packet;
                packet.setData("fileDir", Common::String(filePath.c_str()));
                CommandManager::instance().call(EventId::MeshImport, Common::move(packet));
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                { /* ... */
                    LOG(INFO) << "open";

                    // 1. 触发打开对话框
                    IGFD::FileDialogConfig config;
                    config.path = ".";
                    ImGuiFileDialog::Instance()->OpenDialog("文件选择", "请选择文件", ".msh",
                                                            config);
                }

                // 2. 在渲染循环中显示对话框并获取结果

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
                ImGui::MenuItem("Wireframe", NULL, &showWireframe);
                ImGui::MenuItem("Grid", NULL, &showGrid);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // ===== 工具栏 =====
        ImGui::Begin("##Toolbar", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
        {
            // 分组 1: 文件
            ImGui::BeginGroup();
            ImGui::Text("File");
            ImGui::Separator();
            if (ImGui::SmallButton("Open"))
            { /* ... */
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Save"))
            { /* ... */
            }
            ImGui::EndGroup();

            // 竖线分隔（兼容写法）
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "|");
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();

            // 分组 2: 视图
            ImGui::BeginGroup();
            ImGui::Text("View");
            ImGui::Separator();
            if (ImGui::SmallButton("Wireframe"))
            {
                showWireframe = !showWireframe;
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Grid"))
            {
                showGrid = !showGrid;
            }
            ImGui::EndGroup();

            // 竖线分隔
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "|");
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::SameLine();

            // 分组 3: 相机
            ImGui::BeginGroup();
            ImGui::Text("Camera");
            ImGui::Separator();
            if (ImGui::SmallButton("Fit View"))
            { /* ... */
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Reset"))
            { /* ... */
            }
            ImGui::EndGroup();
        }
        ImGui::End();
    }

} // namespace EgLab::Platform