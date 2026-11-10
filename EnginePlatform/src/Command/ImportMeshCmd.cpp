#include "Command/ImportMeshCmd.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>

#include "Common/Log.hpp"
#include "DataBase/Context.hpp"
#include "DataBase/DataNodeFactory.hpp"
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
        const Common::String meshName(meshPath.filename().string().c_str());
        if (extension == ".msh")
        {
            EgLab::ME::GmshImporter importer(getParams().fileDir);
            mesh3d = importer.getMesh(3);
            mesh2d = importer.getMesh(2);
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

        // A gmsh file can contain nodes for every dimension it references while only
        // providing elements for some of them (e.g. a pure surface mesh still lists the
        // volume nodes). Treat a dimension without elements as absent so that we never
        // create an empty mesh node. An empty 3D node would otherwise become the only
        // locatable target and make locate/render operations see zero elements.
        if (mesh3d && mesh3d->getElemNumber() == 0)
        {
            mesh3d = nullptr;
        }
        if (mesh2d && mesh2d->getElemNumber() == 0)
        {
            mesh2d = nullptr;
        }

        if (!mesh3d && !mesh2d)
        {
            return Common::Return::Failed;
        }

        auto& context = Context::instance();
        auto& factory = DataNodeFactory::instance();

        Common::SharedPtr<MeshData> mesh3dNode = nullptr;
        if (mesh3d)
        {
            mesh3dNode = factory.create<MeshData>(meshName, mesh3d);
            if (context.addDataNode(mesh3dNode) != Common::Return::Succeed)
            {
                return Common::Return::Failed;
            }
        }

        if (mesh2d)
        {
            const Common::String surfaceName =
                mesh3d ? Common::String("2D Surface: ") + meshName : meshName;
            auto node = factory.create<MeshData>(surfaceName, mesh2d);
            const Common::Return result =
                mesh3dNode ? context.getDataTree().addNode(node, mesh3dNode->getId())
                           : context.addDataNode(node);
            if (result != Common::Return::Succeed)
            {
                return Common::Return::Failed;
            }
        }

        if (mesh3d)
        {
            RenderWork::instance().onUpdateMesh(mesh3d);
        }
        if (mesh2d)
        {
            RenderWork::instance().onUpdateMesh(mesh2d);
        }

        LOG(INFO) << "ImportMeshCmd::exec() imported mesh and queued it for rendering";
        return Common::Return::Succeed;
    }

} // namespace EgLab::Platform
