#pragma once

#include "Command/CommandParam.hpp"
#include "Common/Protocol.hpp"
#include "Common/Publisher.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Singleton.hpp"
#include "DataBase/DataNode.hpp"
#include "DataBase/GeometryDataResultPOD.hpp"
#include "DataBase/MeshDataResultPOD.hpp"

namespace EgLab::Platform
{
    class MeshData;
    class GeometryData;

    class MeshDataQueryService : public Common::Singleton<MeshDataQueryService>
    {
    public:
        Common::Return queryAndPublish(DataNodeId dataNodeId);

        // 精确查询：数据树节点清单（一次返回全部节点）。
        Common::Return queryListDataTree();
        // 精确查询：网格节点 id 列表（一次返回全部）。
        Common::Return queryMeshNodes(const MeshQueryParam& param);
        // 精确查询：网格单元 id 列表（一次返回全部）。
        Common::Return queryMeshElements(const MeshQueryParam& param);
        // 精确查询：按节点 id 查询坐标。
        Common::Return queryNodesXYZ(const MeshNodesQueryParam& param);
        // 精确查询：按单元 id 查询拓扑（类型 + 节点 id）。
        Common::Return queryElementsTopology(const MeshElementsQueryParam& param);

        // 几何特征查询（GeometryData）。结果复用同一 MeshDataResult-<pid> 队列，
        // channel 分别为 GetGeometryInfo / ListGeometryFeatures / GeometryImport。
        Common::Return queryGeometryInfo(const GeometryQueryParam& param);
        Common::Return queryGeometryFeatures(const GeometryFeaturesParam& param);
        Common::Return publishGeometryImport(const GeometryImportResult& result);

    private:
        MeshDataQueryService();
        friend class Common::Singleton<MeshDataQueryService>;

        Common::Return fillEntry(const Common::SharedPtr<MeshData>& meshData,
                                 MeshResultEntry& entry);
        Common::Return publishResult(const GetMeshDataResult& result);

        // 按 dataNodeId 或 name 解析 MeshData；失败返回 nullptr。
        Common::SharedPtr<MeshData> resolveMesh(uint64_t dataNodeId, const char* name) const;
        // 按 dataNodeId 或 name 解析 GeometryData；失败返回 nullptr。
        Common::SharedPtr<GeometryData> resolveGeometry(uint64_t dataNodeId, const char* name) const;
        Common::Return publishResult(uint32_t eventId, const void* data, uint32_t size);

        Common::MemoryQueue _resultQueue;
        Common::Publisher _publisher;
    };
} // namespace EgLab::Platform
