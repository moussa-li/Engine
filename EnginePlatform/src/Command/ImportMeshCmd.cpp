#include "Command/ImportMeshCmd.hpp"

#include "Common/Log.hpp"
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

        RenderWork::instance().onUpdateMesh(mesh);

        LOG(INFO) << "ImportMeshCmd::exec() imported mesh and queued it for rendering";
        return Common::Return::Succeed;
    }

} // namespace EgLab::Platform
