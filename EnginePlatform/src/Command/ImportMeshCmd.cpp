#include "Command/ImportMeshCmd.hpp"

#include <filesystem>

#include "Common/Log.hpp"
#include "DataBase/Context.hpp"
#include "MeshEngine/IO/GmshImporter.hpp"
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
        EgLab::ME::GmshImporter importer(getParams().fileDir);
        Common::SharedPtr<ME::Mesh> mesh = importer.getMesh();
        if (mesh == nullptr)
        {
            return Common::Return::Failed;
        }

        const std::filesystem::path meshPath(getParams().fileDir);
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
