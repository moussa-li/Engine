#include "Command/ImportMeshCmd.hpp"

#include <cstring>

#include "Common/Log.hpp"
#include "MeshEngine/IO/GmshImporter.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "MeshEngine/MeshData/MeshPOD.hpp"
#include "MeshEngine/MeshData/MeshToPOD.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    namespace
    {
        struct MeshPODWireHeader
        {
            uint32_t nodeNumber{0};
            uint32_t elemNumber{0};
            uint32_t nodeIdsOffset{0};
            uint32_t nodeDataOffset{0};
            uint32_t elemIdsOffset{0};
            uint32_t elemTypesOffset{0};
            uint32_t elemDataOffset{0};
            uint32_t totalDataSize{0};
        };
    }

    ImportMeshCmd::ImportMeshCmd(const ImportMeshParam& params)
        : CommandBase<ImportMeshParam>(params)
    {
    }

    ImportMeshCmd::~ImportMeshCmd()
    {
    }

    Common::Return ImportMeshCmd::exec()
    {
        EgLab::ME::GmshImporter importer(getParams().fileDir);
        Common::SharedPtr<ME::Mesh> mesh = importer.getMesh();
        if (mesh == nullptr)
        {
            return Common::Return::Failed;
        }

        // ImportMeshCmd is responsible only for file parsing and mesh extraction.
        // After materializing the mesh object, it intentionally emits a nested
        // UpdateMesh command event so the command chain can continue in the
        // same CommandManager event lane.
        const auto pod = EgLab::ME::MeshToPOD::serialize(mesh);
        if (pod.totalDataSize == 0 || pod.data == nullptr)
        {
            LOG(ERROR) << "ImportMeshCmd::exec() MeshToPOD::serialize produced empty payload";
            return Common::Return::Failed;
        }

        // Carry a wire-safe header that mirrors MeshPOD's semantic fields
        // and then append the raw mesh payload bytes. Do not copy the
        // MeshPOD object, because it contains the data pointer field.
        MeshPODWireHeader wire{};
        wire.nodeNumber = pod.nodeNumber;
        wire.elemNumber = pod.elemNumber;
        wire.nodeIdsOffset = pod.nodeIdsOffset;
        wire.nodeDataOffset = pod.nodeDataOffset;
        wire.elemIdsOffset = pod.elemIdsOffset;
        wire.elemTypesOffset = pod.elemTypesOffset;
        wire.elemDataOffset = pod.elemDataOffset;
        wire.totalDataSize = pod.totalDataSize;

        const uint32_t wireSize = static_cast<uint32_t>(sizeof(MeshPODWireHeader));
        UpdateMeshParam updateParam{};
        updateParam.dataSize = wireSize + pod.totalDataSize;
        updateParam.data = new uint8_t[updateParam.dataSize];
        std::memcpy(updateParam.data, &wire, wireSize);
        std::memcpy(updateParam.data + wireSize, pod.data, pod.totalDataSize);

        RenderWork::instance().onUpdateMesh(updateParam);
        delete[] updateParam.data;

        LOG(INFO) << "ImportMeshCmd::exec() imported mesh file and queued it for rendering";
        return Common::Return::Succeed;
    }

} // namespace EgLab::Platform
