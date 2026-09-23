#include "Panel/BackGroundSettingPanel.hpp"

#include "DataBase/Context.hpp"
#include "Work/RenderWork.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    BackGroundSettingPanel::BackGroundSettingPanel()
    {
    }

    BackGroundSettingPanel::~BackGroundSettingPanel()
    {
    }

    void BackGroundSettingPanel::render()
    {
        ImGui::SetNextWindowSize(ImVec2(320, 480), ImGuiCond_FirstUseEver);
        ImGui::Begin("BackGround Settings", &_show);

        ImGui::Text("Appearance");
        ImGui::Separator();

        // 获取当前颜色的引用
        // ImVec4& bgColor = themeMgr.GetWindowBgColorRef();
        auto render = Context::instance().getApplication().renderer;
        auto color = render->getBackGroundColor();
        ImVec4 bgColor(color[0], color[1], color[2], color[3]);

        // 使用 ColorEdit4 编辑颜色
        // ImGuiColorEditFlags_NoInputs: 不显示 RGB 输入框，只显示颜色块
        // ImGuiColorEditFlags_AlphaBar: 显示透明度条
        if (ImGui::ColorEdit4("Window Background", (float*)&bgColor,
                              ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar))
        {
            // 当颜色改变时，立即应用
            // themeMgr.ApplyColors();
        }

        // 预设颜色按钮（可选）
        ImGui::Text("Presets:");
        if (ImGui::Button("Dark"))
        {
            bgColor = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
            // themeMgr.ApplyColors();
        }
        ImGui::SameLine();
        if (ImGui::Button("Light"))
        {
            bgColor = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
            // themeMgr.ApplyColors();
        }
        ImGui::SameLine();
        if (ImGui::Button("Red Alert"))
        {
            bgColor = ImVec4(0.5f, 0.1f, 0.1f, 1.0f);
            // themeMgr.ApplyColors();
        }

        color = Common::Vector4f(bgColor.x, bgColor.y, bgColor.z, bgColor.w);
        render->setBackGroundColor(color);

        ImGui::End();
    }

} // namespace EgLab::Platform