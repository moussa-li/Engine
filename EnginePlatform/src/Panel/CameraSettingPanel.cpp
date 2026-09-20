#include "Panel/CameraSettingPanel.hpp"

#include "DataBase/Context.hpp"
#include "Work/RenderWork.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    CameraSettingPanel::CameraSettingPanel()
    {
    }

    CameraSettingPanel::~CameraSettingPanel()
    {
    }

    void CameraSettingPanel::render()
    {
        bool show = isShow();
        ImGui::SetNextWindowSize(ImVec2(320, 480), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Camera Setting", &_show))
        {
            ImGui::Text("Camera Setting Panel");
            ImGui::Separator();

            // ========== Camera Position ==========
            ImGui::Text("Camera Position");
            static float camPos[3] = {0.0f, 5.0f, 10.0f};
            ImGui::PushItemWidth(70);
            ImGui::InputFloat3("##CamPos", camPos, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "XYZ");

            // ========== Camera Target ==========
            ImGui::Text("Camera Target");
            static float camTarget[3] = {0.0f, 0.0f, 0.0f};
            ImGui::PushItemWidth(70);
            ImGui::InputFloat3("##CamTarget", camTarget, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "XYZ");

            ImGui::Separator();

            // ========== Sensitivity ==========
            ImGui::Text("Sensitivity");
            static float moveSens = 1.0f;
            static float rotateSens = 1.0f;
            static float zoomSens = 1.0f;

            ImGui::SliderFloat("Move Sensitivity", &moveSens, 0.1f, 5.0f, "%.2f");
            ImGui::SliderFloat("Rotate Sensitivity", &rotateSens, 0.1f, 5.0f, "%.2f");
            ImGui::SliderFloat("Zoom Sensitivity", &zoomSens, 0.1f, 5.0f, "%.2f");

            ImGui::Separator();

            // ========== Fit View Button ==========
            if (ImGui::Button("Fit View", ImVec2(-1, 0)))
            {
                RenderWork::instance().fitView();
            }

            ImGui::SameLine();
            if (ImGui::Button("Reset All", ImVec2(-1, 0)))
            {
                // 重置所有参数
                camPos[0] = 0.0f;
                camPos[1] = 5.0f;
                camPos[2] = 10.0f;
                camTarget[0] = 0.0f;
                camTarget[1] = 0.0f;
                camTarget[2] = 0.0f;
                moveSens = 1.0f;
                rotateSens = 1.0f;
                zoomSens = 1.0f;
            }

            ImGui::Separator();

            // ========== Close Button ==========
            ImGui::Spacing();
            if (ImGui::Button("Close", ImVec2(-1, 0)))
            {
                close();
            }
        }
        ImGui::End();
    }
} // namespace EgLab::Platform