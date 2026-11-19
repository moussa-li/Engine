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

    Common::Return MeshDataQueryService::publishResult(uint32_t eventId, const void* data,
                                                       uint32_t size)
    {
        return _resultQueue.push(eventId, data, size) ? Common::Return::Succeed
                                                      : Common::Return::Failed;
    }

    Common::SharedPtr<MeshData> MeshDataQueryService::resolveMesh(uint64_t dataNodeId,
                                                                  const char* name) const
    {
        auto& context = Context::instance();
        Common::SharedPtr<DataNode> node;
        if (dataNodeId != 0)
        {
            node = context.getDataNode(dataNodeId);
        }
        else if (name != nullptr && name[0] != '\0')
        {
            node = context.getDataNode(Common::String(name));
        }
        return Common::dynamicSharedPtrCast<MeshData>(node);
    }

    Common::Return MeshDataQueryService::queryListDataTree()
    {
        const auto dataNodes = Context::instance().getDataTree().getDataNodes();

        MeshQueryResultHeader header{};
        header.magic = MESH_RESULT_MAGIC;
        header.version = MESH_RESULT_VERSION;
        header.status = 0;
        header.total = static_cast<uint32_t>(dataNodes.size());
        header.offset = 0;
        header.returned = header.total;

        Common::DynamicArray<uint8_t> buffer;
        buffer.resize(sizeof(header) + static_cast<size_t>(header.returned) *
                                           sizeof(DataTreeNodeEntry));
        std::memcpy(buffer.data(), &header, sizeof(header));

        uint8_t* cursor = buffer.data() + sizeof(header);
        for (size_t index = 0; index < dataNodes.size(); ++index)
        {
            DataTreeNodeEntry entry{};
            const auto& node = dataNodes[index];
            entry.dataNodeId = node->getId();
            const Common::String& name = node->getName();
            const size_t copySize =
                name.size() < sizeof(entry.name) - 1 ? name.size() : sizeof(entry.name) - 1;
            std::memcpy(entry.name, name.c_str(), copySize);
            entry.name[copySize] = '\0';
            entry.reserved = 0;
            entry.nodeCount = 0;
            entry.elemCount = 0;

            auto meshData = Common::dynamicSharedPtrCast<MeshData>(node);
            if (meshData)
            {
                entry.kind = 1;
                auto mesh = meshData->getMesh();
                if (mesh)
                {
                    entry.nodeCount = static_cast<uint64_t>(mesh->getNodeNumber());
                    entry.elemCount = static_cast<uint64_t>(mesh->getElemNumber());
                }
            }
            else
            {
                entry.kind = 0;
            }

            std::memcpy(cursor, &entry, sizeof(entry));
            cursor += sizeof(entry);
        }

        return publishResult(static_cast<uint32_t>(EventId::ListDataTreeNodes), buffer.data(),
                             static_cast<uint32_t>(buffer.size()));
    }

    Common::Return MeshDataQueryService::queryMeshNodes(const MeshQueryParam& param)
    {
        MeshQueryResultHeader header{};
        header.magic = MESH_RESULT_MAGIC;
        header.version = MESH_RESULT_VERSION;
        header.status = 0;
        header.offset = 0;

        auto meshData = resolveMesh(param.dataNodeId, param.name);
        Common::SharedPtr<ME::Mesh> mesh;
        if (meshData)
        {
            mesh = meshData->getMesh();
        }
        if (!mesh)
        {
            header.status = 1;
            return publishResult(static_cast<uint32_t>(EventId::GetMeshNodes), &header,
                                 sizeof(header));
        }

        header.total = static_cast<uint32_t>(mesh->getNodeNumber());
        header.returned = header.total;

        Common::DynamicArray<uint8_t> buffer;
        buffer.resize(sizeof(header) + static_cast<size_t>(header.returned) * sizeof(uint32_t));
        std::memcpy(buffer.data(), &header, sizeof(header));

        uint32_t* ids = reinterpret_cast<uint32_t*>(buffer.data() + sizeof(header));
        uint32_t written = 0;
        ME::MeshIterator iterator(*mesh);
        for (; iterator.hasMoreNode(); iterator.nextNode())
        {
            ids[written++] = iterator.currentNode().getId();
        }

        return publishResult(static_cast<uint32_t>(EventId::GetMeshNodes), buffer.data(),
                             static_cast<uint32_t>(buffer.size()));
    }

    Common::Return MeshDataQueryService::queryMeshElements(const MeshQueryParam& param)
    {
        MeshQueryResultHeader header{};
        header.magic = MESH_RESULT_MAGIC;
        header.version = MESH_RESULT_VERSION;
        header.status = 0;
        header.offset = 0;

        auto meshData = resolveMesh(param.dataNodeId, param.name);
        Common::SharedPtr<ME::Mesh> mesh;
        if (meshData)
        {
            mesh = meshData->getMesh();
        }
        if (!mesh)
        {
            header.status = 1;
            return publishResult(static_cast<uint32_t>(EventId::GetMeshElements), &header,
                                 sizeof(header));
        }

        header.total = static_cast<uint32_t>(mesh->getElemNumber());
        header.returned = header.total;

        Common::DynamicArray<uint8_t> buffer;
        buffer.resize(sizeof(header) + static_cast<size_t>(header.returned) * sizeof(uint32_t));
        std::memcpy(buffer.data(), &header, sizeof(header));

        uint32_t* ids = reinterpret_cast<uint32_t*>(buffer.data() + sizeof(header));
        uint32_t written = 0;
        ME::MeshIterator iterator(*mesh);
        for (; iterator.hasMoreElem(); iterator.nextElem())
        {
            ids[written++] = iterator.currentElem().getId();
        }

        return publishResult(static_cast<uint32_t>(EventId::GetMeshElements), buffer.data(),
                             static_cast<uint32_t>(buffer.size()));
    }

    Common::Return MeshDataQueryService::queryNodesXYZ(const MeshNodesQueryParam& param)
    {
        MeshQueryResultHeader header{};
        header.magic = MESH_RESULT_MAGIC;
        header.version = MESH_RESULT_VERSION;
        header.status = 0;
        header.offset = 0;

        auto meshData = resolveMesh(param.dataNodeId, param.name);
        Common::SharedPtr<ME::Mesh> mesh;
        if (meshData)
        {
            mesh = meshData->getMesh();
        }
        if (!mesh)
        {
            header.status = 1;
            return publishResult(static_cast<uint32_t>(EventId::GetNodesXYZ), &header,
                                 sizeof(header));
        }

        Common::DynamicArray<uint8_t> buffer;
        buffer.resize(sizeof(header) + static_cast<size_t>(param.count) * sizeof(NodeXYZEntry));
        NodeXYZEntry* entries =
            reinterpret_cast<NodeXYZEntry*>(buffer.data() + sizeof(header));

        uint32_t written = 0;
        for (uint32_t index = 0; index < param.count; ++index)
        {
            const ME::IdType nodeId = param.nodeIds[index];
            if (mesh->getNodeIdx(nodeId) == ME::INVALID_IDX)
            {
                continue;
            }
            ME::Node& node = mesh->getNodeById(nodeId);
            NodeXYZEntry& entry = entries[written++];
            entry.id = nodeId;
            entry.xyz[0] = static_cast<float>(node.x());
            entry.xyz[1] = static_cast<float>(node.y());
            entry.xyz[2] = static_cast<float>(node.z());
        }

        header.total = written;
        header.returned = written;
        std::memcpy(buffer.data(), &header, sizeof(header));

        const uint32_t payloadSize =
            static_cast<uint32_t>(sizeof(header) + static_cast<size_t>(written) *
                                                      sizeof(NodeXYZEntry));
        return publishResult(static_cast<uint32_t>(EventId::GetNodesXYZ), buffer.data(),
                             payloadSize);
    }

    Common::Return MeshDataQueryService::queryElementsTopology(
        const MeshElementsQueryParam& param)
    {
        MeshQueryResultHeader header{};
        header.magic = MESH_RESULT_MAGIC;
        header.version = MESH_RESULT_VERSION;
        header.status = 0;
        header.offset = 0;

        auto meshData = resolveMesh(param.dataNodeId, param.name);
        Common::SharedPtr<ME::Mesh> mesh;
        if (meshData)
        {
            mesh = meshData->getMesh();
        }
        if (!mesh)
        {
            header.status = 1;
            return publishResult(static_cast<uint32_t>(EventId::GetElementsTopology), &header,
                                 sizeof(header));
        }

        Common::DynamicArray<uint8_t> buffer;
        buffer.resize(sizeof(header) + static_cast<size_t>(param.count) * sizeof(ElementTopoEntry));
        ElementTopoEntry* entries =
            reinterpret_cast<ElementTopoEntry*>(buffer.data() + sizeof(header));

        uint32_t written = 0;
        for (uint32_t index = 0; index < param.count; ++index)
        {
            const ME::IdType elemId = param.elemIds[index];
            if (mesh->getElemIdx(elemId) == ME::INVALID_IDX)
            {
                continue;
            }
            ME::Elem& elem = mesh->getElemById(elemId);
            ElementTopoEntry& entry = entries[written++];
            entry.id = elemId;
            entry.type = static_cast<uint32_t>(elem.getType());

            const auto& nodes = elem.getNodes();
            const uint32_t nodeCount = nodes.size() > MAX_ELEM_NODE_COUNT
                                           ? MAX_ELEM_NODE_COUNT
                                           : static_cast<uint32_t>(nodes.size());
            entry.nodeCount = nodeCount;
            for (uint32_t n = 0; n < nodeCount; ++n)
            {
                entry.nodeIds[n] = nodes[n];
            }
        }

        header.total = written;
        header.returned = written;
        std::memcpy(buffer.data(), &header, sizeof(header));

        const uint32_t payloadSize =
            static_cast<uint32_t>(sizeof(header) + static_cast<size_t>(written) *
                                                      sizeof(ElementTopoEntry));
        return publishResult(static_cast<uint32_t>(EventId::GetElementsTopology), buffer.data(),
                             payloadSize);
    }
} // namespace EgLab::Platform
