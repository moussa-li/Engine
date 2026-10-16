#include "Panel/LightSettingPanel.hpp"

#include "DataBase/Context.hpp"
#include "RenderEngine/Core/Renderer.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    void LightSettingPanel::render()
    {
        ImGui::SetNextWindowSize(ImVec2(340, 300), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Light Settings", &_show))
        {
            auto renderer = Context::instance().getApplication().renderer;
            if (renderer)
            {
                auto settings = renderer->getLightSettings();
                float ambient[3] = {settings.ambient[0], settings.ambient[1], settings.ambient[2]};
                float diffuse[3] = {settings.diffuse[0], settings.diffuse[1], settings.diffuse[2]};
                float specular[3] = {settings.specular[0], settings.specular[1],
                                     settings.specular[2]};

                bool changed = ImGui::SliderFloat3("Ambient", ambient, 0.0f, 1.0f, "%.2f");
                changed |= ImGui::SliderFloat3("Diffuse", diffuse, 0.0f, 1.0f, "%.2f");
                changed |= ImGui::SliderFloat3("Specular", specular, 0.0f, 1.0f, "%.2f");
                changed |= ImGui::SliderFloat("Shininess", &settings.shininess, 1.0f, 256.0f,
                                              "%.0f", ImGuiSliderFlags_Logarithmic);

                if (changed)
                {
                    settings.ambient = Common::Vector3f(ambient[0], ambient[1], ambient[2]);
                    settings.diffuse = Common::Vector3f(diffuse[0], diffuse[1], diffuse[2]);
                    settings.specular = Common::Vector3f(specular[0], specular[1], specular[2]);
                    renderer->setLightSettings(settings);
                }
            }
        }
        ImGui::End();
    }
} // namespace EgLab::Platform