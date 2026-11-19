#pragma once

#include "Common/Protocol.hpp"
#include "Common/Publisher.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Singleton.hpp"
#include "DataBase/DataNode.hpp"
#include "DataBase/MeshDataResultPOD.hpp"

namespace EgLab::Platform
{
    class MeshData;

    class MeshDataQueryService : public Common::Singleton<MeshDataQueryService>
    {
    public:
        Common::Return queryAndPublish(DataNodeId dataNodeId);

    private:
        MeshDataQueryService();
        friend class Common::Singleton<MeshDataQueryService>;

        Common::Return fillEntry(const Common::SharedPtr<MeshData>& meshData,
                                 MeshResultEntry& entry);
        Common::Return publishResult(const GetMeshDataResult& result);

        Common::MemoryQueue _resultQueue;
        Common::Publisher _publisher;
    };
} // namespace EgLab::Platform
