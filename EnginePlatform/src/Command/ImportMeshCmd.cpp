#include "Command/ImportMeshCmd.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>

#include "Common/Log.hpp"
#include "DataBase/Context.hpp"
#include "DataBase/MeshData.hpp"
#include "MeshEngine/IO/GmshImporter.hpp"
#include "MeshEngine/IO/GrdImporter.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    ImportMeshCmd::ImportMeshCmd(const ImportMeshParam& params)
        : CommandBase<ImportMeshParam>(params)
    {
    }

    ImportMeshCmd::~ImportMeshCmd()
    {
    }

    Common::Return ImportMeshCmd::execImpl()
    {
        const std::filesystem::path meshPath(getParams().fileDir);
        std::string extension = meshPath.extension().string();
        std::transform(
            extension.begin(), extension.end(), extension.begin(),
            [](unsigned char character) { return static_cast<char>(std::tolower(character)); });

        Common::SharedPtr<ME::Mesh> mesh3d = nullptr;
        Common::SharedPtr<ME::Mesh> mesh2d = nullptr;
        MeshData* meshData = new MeshData(meshPath.filename().string().c_str(), nullptr);
        if (extension == ".msh")
        {
            EgLab::ME::GmshImporter importer(getParams().fileDir);
            mesh3d = importer.getMesh(3);
            meshData->getMesh() = mesh3d;
            mesh2d = importer.getMesh(2);
            auto meshData2d = new MeshData("2D Surface", mesh2d);
            meshData->addTree(meshData2d);
        }
        else if (extension == ".grd")
        {
            EgLab::ME::GrdImporter importer(getParams().fileDir);
            mesh2d = importer.getMesh(2);
        }
        else
        {
            LOG(ERROR) << "Unsupported mesh file extension: " << extension;
            return Common::Return::BadInput;
        }

        if (meshData == nullptr)
        {
            return Common::Return::Failed;
        }

        const Common::String meshName(meshPath.filename().string().c_str());
        if (Context::instance().getDataTree().addTree(meshData) != Common::Return::Succeed)
        {
            LOG(ERROR) << "ImportMeshCmd::exec() failed to register imported mesh data";
            return Common::Return::Failed;
        }

        RenderWork::instance().onUpdateMesh(mesh3d);
        RenderWork::instance().onUpdateMesh(mesh2d);

        LOG(INFO) << "ImportMeshCmd::exec() imported mesh and queued it for rendering";
        return Common::Return::Succeed;
    }

} // namespace EgLab::Platform
