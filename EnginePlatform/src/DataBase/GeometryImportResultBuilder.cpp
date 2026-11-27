#include "DataBase/GeometryImportResultBuilder.hpp"

#include "DataBase/GeometryData.hpp"
#include "DataBase/GeometryMeshBuild.hpp"
#include "GeometryEngine/Model/GeometryDocument.hpp"
#include "GeometryEngine/Topology/TopologyType.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "MeshEngine/MeshData/Node.hpp"

namespace EgLab::Platform
{
    namespace
    {
        void fillMeshBBox(const ME::Mesh& mesh, float bboxMin[3], float bboxMax[3])
        {
            bool hasNode = false;
            ME::MeshIterator iterator(const_cast<ME::Mesh&>(mesh));
            for (; iterator.hasMoreNode(); iterator.nextNode())
            {
                const ME::Node& node = iterator.currentNode();
                const float x = static_cast<float>(node.x());
                const float y = static_cast<float>(node.y());
                const float z = static_cast<float>(node.z());
                if (!hasNode)
                {
                    bboxMin[0] = bboxMax[0] = x;
                    bboxMin[1] = bboxMax[1] = y;
                    bboxMin[2] = bboxMax[2] = z;
                    hasNode = true;
                    continue;
                }
                if (x < bboxMin[0]) bboxMin[0] = x;
                if (y < bboxMin[1]) bboxMin[1] = y;
                if (z < bboxMin[2]) bboxMin[2] = z;
                if (x > bboxMax[0]) bboxMax[0] = x;
                if (y > bboxMax[1]) bboxMax[1] = y;
                if (z > bboxMax[2]) bboxMax[2] = z;
            }
        }
    } // namespace

    GeometryImportResult GeometryImportResultBuilder::makeFailure(uint32_t status)
    {
        GeometryImportResult result{};
        result.magic = GEOM_RESULT_MAGIC;
        result.version = GEOM_RESULT_VERSION;
        result.status = status;
        return result;
    }

    GeometryImportResult GeometryImportResultBuilder::makeSuccess(const GeometryData& data,
                                                                  const GeometryMeshBuild& build)
    {
        GeometryImportResult result = makeFailure(0);
        result.dataNodeId = data.getId();
        result.vertexCount =
            static_cast<uint32_t>(data.getFeatureCount(GE::TopologyType::Vertex));
        result.edgeCount = static_cast<uint32_t>(data.getFeatureCount(GE::TopologyType::Edge));
        result.faceCount = static_cast<uint32_t>(data.getFeatureCount(GE::TopologyType::Face));
        result.solidCount = static_cast<uint32_t>(data.getFeatureCount(GE::TopologyType::Solid));
        result.triangleCount = build.triangleCount;
        if (build.mesh)
        {
            fillMeshBBox(*build.mesh, result.bboxMin, result.bboxMax);
        }
        return result;
    }
} // namespace EgLab::Platform
