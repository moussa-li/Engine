#include "Panel/MeshImportPanel.hpp"

#include "Core/Application.hpp"
#include "ImGuiFileDialog.h"
#include "MeshEngine/IO/GmshImporter.hpp"
#include "RenderEngine/Core/MeshPrimitiveCreator.hpp"
#include "RenderEngine/Core/RenderFace.hpp"
#include "RenderEngine/Core/RenderLine.hpp"
#include "RenderEngine/Core/RenderNode.hpp"
#include "RenderEngine/Core/Scene.hpp"
#include "RenderEngine/Core/ShaderLib.hpp"

namespace EgLab::Platform
{
    MeshImportPanel::MeshImportPanel()
    {
    }

    void MeshImportPanel::render()
    {
        if (ImGuiFileDialog::Instance()->Display("文件选择"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                EgLab::ME::GmshImporter importer(filePath.c_str());
                Common::SharedPtr<ME::Mesh> mesh;
                mesh = importer.getMesh();
                if (mesh == nullptr) return;
                EgLab::RE::MeshPrimitiveCreator creator(mesh);
                auto nodePrimitive = creator.getPrimitive<EgLab::RE::RenderNode>();
                auto linePrimitive = creator.getPrimitive<EgLab::RE::RenderLine>();
                auto facePrimitive = creator.getPrimitive<EgLab::RE::RenderFace>();
                Application* app;
                return;

                EgLab::Common::SharedPtr<EgLab::RE::Shader> shader;
                EgLab::RE::ShaderLib::instance().getNodeShader(shader);
                app->scene->addPrimitive(shader, nodePrimitive);

                EgLab::RE::ShaderLib::instance().getLineShader(shader);
                app->scene->addPrimitive(shader, linePrimitive);

                EgLab::RE::ShaderLib::instance().getFaceShader(shader);
                app->scene->addPrimitive(shader, facePrimitive);
            }
            ImGuiFileDialog::Instance()->Close();

            // this->hide();
        }
    }

} // namespace EgLab::Platform