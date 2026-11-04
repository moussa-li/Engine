#include "Command/UpdateMeshCmd.hpp"

#include <cstring>

#include "Command/MeshPODWire.hpp"
#include "Common/Log.hpp"
#include "DataBase/Context.hpp"
#include "DataBase/DataNodeFactory.hpp"
#include "DataBase/MeshData.hpp"
#include "MeshEngine/MeshData/MeshPOD.hpp"
#include "MeshEngine/MeshData/MeshToPOD.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    UpdateMeshCmd::UpdateMeshCmd(const UpdateMeshParam& params)
        : CommandBase<UpdateMeshParam>(params)
    {
    }

    UpdateMeshCmd::~UpdateMeshCmd()
    {
        if (getParams().data != nullptr)
        {
            delete[] getParams().data;
        }
    }

    Common::Return UpdateMeshCmd::execImpl()
    {
        const auto& params = getParams();
        if (params.dataSize < sizeof(MeshPODWireHeader) || params.data == nullptr)
        {
            return Common::Return::BadInput;
        }

        MeshPODWireHeader wire{};
        std::memcpy(&wire, params.data, sizeof(wire));
        const char* nameEnd =
            static_cast<const char*>(std::memchr(wire.meshName, '\0', sizeof(wire.meshName)));
        if (nameEnd == nullptr || nameEnd == wire.meshName || wire.totalDataSize == 0 ||
            wire.totalDataSize > params.dataSize - sizeof(MeshPODWireHeader))
        {
            return Common::Return::BadInput;
        }

        const uint32_t offsets[] = {
            wire.nodeIdsOffset,   wire.nodeDataOffset, wire.elemIdsOffset,
            wire.elemTypesOffset, wire.elemDataOffset,
        };
        for (const uint32_t offset : offsets)
        {
            if (offset > wire.totalDataSize)
            {
                return Common::Return::BadInput;
            }
        }

        EgLab::ME::MeshPOD pod{};
        pod.nodeNumber = wire.nodeNumber;
        pod.elemNumber = wire.elemNumber;
        pod.nodeIdsOffset = wire.nodeIdsOffset;
        pod.nodeDataOffset = wire.nodeDataOffset;
        pod.elemIdsOffset = wire.elemIdsOffset;
        pod.elemTypesOffset = wire.elemTypesOffset;
        pod.elemDataOffset = wire.elemDataOffset;
        pod.totalDataSize = wire.totalDataSize;
        pod.data = new uint8_t[pod.totalDataSize];
        std::memcpy(pod.data, params.data + sizeof(MeshPODWireHeader), pod.totalDataSize);

        auto mesh = EgLab::ME::MeshToPOD::deserialize(pod);
        if (!mesh)
        {
            return Common::Return::Failed;
        }

        const Common::String meshName(wire.meshName);
        auto& context = Context::instance();
        auto existingNode = context.getDataNode(meshName);
        Common::Vector4f meshColor(0.8f, 0.8f, 0.8f, 1.0f);
        if (existingNode)
        {
            auto* meshData = dynamic_cast<MeshData*>(existingNode.get());
            if (meshData == nullptr)
            {
                LOG(ERROR) << "UpdateMeshCmd::exec() key already belongs to a non-mesh data node";
                return Common::Return::Failed;
            }

            meshColor = meshData->getColor();
            auto previousMesh = meshData->getMesh();
            meshData->setMesh(mesh);
            if (previousMesh)
            {
                RenderWork::instance().removeMesh(previousMesh);
            }
        }
        else
        {
            auto node = DataNodeFactory::instance().create<MeshData>(meshName, mesh);
            if (context.addDataNode(node) != Common::Return::Succeed)
            {
                return Common::Return::Failed;
            }
        }

        RenderWork::instance().onUpdateMesh(mesh);
        if (existingNode)
        {
            RenderWork::instance().setMeshColor(mesh, meshColor);
        }

        LOG(INFO) << "UpdateMeshCmd::exec() updated mesh data and queued it for rendering";
        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform
