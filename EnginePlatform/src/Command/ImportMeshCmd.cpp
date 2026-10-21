#include "Command/ImportMeshCmd.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>

#include "Common/Log.hpp"
#include "DataBase/Context.hpp"
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

        Common::SharedPtr<ME::Mesh> mesh;
        if (extension == ".msh")
        {
            EgLab::ME::GmshImporter importer(getParams().fileDir);
            mesh = importer.getMesh();
        }
        else if (extension == ".grd")
        {
            EgLab::ME::GrdImporter importer(getParams().fileDir);
            mesh = importer.getMesh();
        }
        else
        {
            LOG(ERROR) << "Unsupported mesh file extension: " << extension;
            return Common::Return::BadInput;
        }

        if (mesh == nullptr)
        {
            return Common::Return::Failed;
        }

        const Common::String meshName(meshPath.filename().string().c_str());
        if (Context::instance().addMeshData(mesh, meshName) != Common::Return::Succeed)
        {
            LOG(ERROR) << "ImportMeshCmd::exec() failed to register imported mesh data";
            return Common::Return::Failed;
        }

        RenderWork::instance().onUpdateMesh(mesh);

        LOG(INFO) << "ImportMeshCmd::exec() imported mesh and queued it for rendering";
        return Common::Return::Succeed;
    }

} // namespace EgLab::Platform
