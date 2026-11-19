#include "DataBase/MeshDataQueryService.hpp"

#include <cstring>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

#include "Command/CommandIds.hpp"
#include "Command/CommandQueueName.hpp"
#include "DataBase/Context.hpp"
#include "DataBase/MeshData.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab::Platform
{
    namespace
    {
        uint32_t currentProcessId()
        {
#ifdef _WIN32
            return static_cast<uint32_t>(_getpid());
#else
            return static_cast<uint32_t>(getpid());
#endif
        }
    } // namespace

    MeshDataQueryService::MeshDataQueryService()
        : _resultQueue(meshDataResultMemoryName(currentProcessId()), true),
          _publisher(_resultQueue)
    {
    }

    Common::Return MeshDataQueryService::queryAndPublish(DataNodeId dataNodeId)
    {
        GetMeshDataResult result{};
        result.header.magic = MESH_RESULT_MAGIC;
        result.header.version = MESH_RESULT_VERSION;
        result.header.status = 0;
        result.header.flags = 0;

        if (dataNodeId != 0)
        {
            auto node = Context::instance().getDataNode(dataNodeId);
            auto meshData = Common::dynamicSharedPtrCast<MeshData>(node);
            if (!meshData)
            {
                result.header.status = 1;
                return publishResult(result);
            }

            fillEntry(meshData, result.entries[0]);
            result.header.meshCount = 1;
            result.header.totalNodeCount = result.entries[0].nodeCount;
            result.header.totalElemCount = result.entries[0].elemCount;
            std::memcpy(result.header.totalBBoxMin, result.entries[0].bboxMin,
                        sizeof(result.header.totalBBoxMin));
            std::memcpy(result.header.totalBBoxMax, result.entries[0].bboxMax,
                        sizeof(result.header.totalBBoxMax));
            return publishResult(result);
        }

        const auto dataNodes = Context::instance().getDataTree().getDataNodes();
        bool hasTotalBBox = false;
        for (size_t index = 0; index < dataNodes.size(); ++index)
        {
            auto meshData = Common::dynamicSharedPtrCast<MeshData>(dataNodes[index]);
            if (!meshData)
            {
                continue;
            }

            MeshResultEntry entry{};
            fillEntry(meshData, entry);

            result.header.totalNodeCount += entry.nodeCount;
            result.header.totalElemCount += entry.elemCount;

            if (!hasTotalBBox)
            {
                std::memcpy(result.header.totalBBoxMin, entry.bboxMin,
                            sizeof(result.header.totalBBoxMin));
                std::memcpy(result.header.totalBBoxMax, entry.bboxMax,
                            sizeof(result.header.totalBBoxMax));
                hasTotalBBox = true;
            }
            else
            {
                for (size_t axis = 0; axis < 3; ++axis)
                {
                    if (entry.bboxMin[axis] < result.header.totalBBoxMin[axis])
                    {
                        result.header.totalBBoxMin[axis] = entry.bboxMin[axis];
                    }
                    if (entry.bboxMax[axis] > result.header.totalBBoxMax[axis])
                    {
                        result.header.totalBBoxMax[axis] = entry.bboxMax[axis];
                    }
                }
            }

            if (result.header.meshCount < MAX_MESH_RESULT_ENTRIES)
            {
                result.entries[result.header.meshCount] = entry;
            }
            else
            {
                result.header.flags |= 1u << 0; // truncated
            }
            ++result.header.meshCount;
        }

        return publishResult(result);
    }

    Common::Return MeshDataQueryService::fillEntry(const Common::SharedPtr<MeshData>& meshData,
                                                   MeshResultEntry& entry)
    {
        entry.dataNodeId = meshData->getId();

        const Common::String& name = meshData->getName();
        const size_t copySize = name.size() < sizeof(entry.name) - 1 ? name.size()
                                                                     : sizeof(entry.name) - 1;
        std::memcpy(entry.name, name.c_str(), copySize);
        entry.name[copySize] = '\0';

        auto mesh = meshData->getMesh();
        if (!mesh)
        {
            return Common::Return::Succeed;
        }

        entry.nodeCount = static_cast<uint64_t>(mesh->getNodeNumber());
        entry.elemCount = static_cast<uint64_t>(mesh->getElemNumber());

        bool hasNode = false;
        float min[3] = {0.0f, 0.0f, 0.0f};
        float max[3] = {0.0f, 0.0f, 0.0f};
        ME::MeshIterator nodeIterator(*mesh);
        for (; nodeIterator.hasMoreNode(); nodeIterator.nextNode())
        {
            const ME::Node& node = nodeIterator.currentNode();
            const float x = static_cast<float>(node.x());
            const float y = static_cast<float>(node.y());
            const float z = static_cast<float>(node.z());

            if (!hasNode)
            {
                min[0] = max[0] = x;
                min[1] = max[1] = y;
                min[2] = max[2] = z;
                hasNode = true;
            }
            else
            {
                if (x < min[0]) min[0] = x;
                if (y < min[1]) min[1] = y;
                if (z < min[2]) min[2] = z;
                if (x > max[0]) max[0] = x;
                if (y > max[1]) max[1] = y;
                if (z > max[2]) max[2] = z;
            }
        }

        if (hasNode)
        {
            std::memcpy(entry.bboxMin, min, sizeof(entry.bboxMin));
            std::memcpy(entry.bboxMax, max, sizeof(entry.bboxMax));
        }

        int32_t typeIds[MAX_ELEM_TYPES_PER_MESH] = {};
        uint32_t typeCounts[MAX_ELEM_TYPES_PER_MESH] = {};
        uint32_t typeCount = 0;

        ME::MeshIterator elemIterator(*mesh);
        for (; elemIterator.hasMoreElem(); elemIterator.nextElem())
        {
            const ME::Elem& elem = elemIterator.currentElem();
            const ME::ElemType type = elem.getType();
            if (type == ME::ElemType::None)
            {
                continue;
            }

            const int32_t typeId = static_cast<int32_t>(type);
            uint32_t slot = 0;
            for (; slot < typeCount; ++slot)
            {
                if (typeIds[slot] == typeId)
                {
                    break;
                }
            }

            if (slot == typeCount)
            {
                if (typeCount >= MAX_ELEM_TYPES_PER_MESH)
                {
                    continue;
                }
                typeIds[slot] = typeId;
                typeCounts[slot] = 0;
                ++typeCount;
            }

            ++typeCounts[slot];
        }

        entry.elemTypeCount = typeCount;
        for (uint32_t slot = 0; slot < typeCount; ++slot)
        {
            entry.elemTypeIds[slot] = static_cast<uint32_t>(typeIds[slot]);
            entry.elemTypeCounts[slot] = typeCounts[slot];
        }

        return Common::Return::Succeed;
    }

    Common::Return MeshDataQueryService::publishResult(const GetMeshDataResult& result)
    {
        return _publisher.publish(static_cast<uint32_t>(EventId::GetMeshData), result)
                   ? Common::Return::Succeed
                   : Common::Return::Failed;
    }
} // namespace EgLab::Platform
