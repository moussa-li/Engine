#include "Panel/CameraSettingPanel.hpp"

#include "Command/CommandManager.hpp"
#include "DataBase/Context.hpp"
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
        ImGui::SetNextWindowSize(ImVec2(320, 480), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Camera Setting", &_show))
        {
            auto camera = Context::instance().getApplication().camera;
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
            float moveSens = camera ? camera->getMoveSensitivity() : 1.0f;
            float rotateSens = camera ? camera->getRotateSensitivity() : 1.0f;
            float zoomSens = camera ? camera->getZoomSensitivity() : 1.0f;

            if (ImGui::SliderFloat("Move Sensitivity", &moveSens, 0.1f, 5.0f, "%.2f") && camera)
            {
                camera->setMoveSensitivity(moveSens);
            }
            if (ImGui::SliderFloat("Rotate Sensitivity", &rotateSens, 0.1f, 5.0f, "%.2f") && camera)
            {
                camera->setRotateSensitivity(rotateSens);
            }
            if (ImGui::SliderFloat("Zoom Sensitivity", &zoomSens, 0.1f, 5.0f, "%.2f") && camera)
            {
                camera->setZoomSensitivity(zoomSens);
            }

            ImGui::Separator();

            // ========== Fit View Button ==========
            if (ImGui::Button("Fit View", ImVec2(-1, 0)))
            {
                CommandManager::instance().call(EventId::FitView, EventPacket{});
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
                if (camera)
                {
                    camera->setMoveSensitivity(1.0f);
                    camera->setRotateSensitivity(1.0f);
                    camera->setZoomSensitivity(1.0f);
                }
            }

            ImGui::Separator();
        }
        ImGui::End();
    }
} // namespace EgLab::Platform