#include "MeshEngine/MeshData/MeshToPOD.hpp"

#include <cstring>

#include "MeshEngine/MeshData/Mesh.hpp"
#include "MeshEngine/MeshData/MeshPOD.hpp"

namespace EgLab::ME
{
    namespace
    {
        inline void writePrimitive(uint8_t *buffer, uint32_t offset, const void *value, uint32_t byteSize)
        {
            if (buffer == nullptr || value == nullptr)
            {
                return;
            }
            std::memcpy(buffer + offset, value, byteSize);
        }

        template <typename T>
        inline T readPrimitive(const uint8_t *buffer, uint32_t offset)
        {
            T value{};
            if (buffer != nullptr)
            {
                std::memcpy(&value, buffer + offset, sizeof(T));
            }
            return value;
        }
    } // namespace

    MeshPOD MeshToPOD::serialize(const Common::SharedPtr<Mesh> mesh)
    {
        MeshPOD pod;
        if (!mesh)
        {
            return pod;
        }

        pod.nodeNumber = static_cast<uint32_t>(mesh->getNodeNumber());
        pod.elemNumber = static_cast<uint32_t>(mesh->getElemNumber());

        uint32_t nodeIdsSize = pod.nodeNumber * sizeof(IdType);
        uint32_t nodeDataSize = pod.nodeNumber * sizeof(Scalar) * 3;
        uint32_t elemIdsSize = pod.elemNumber * sizeof(IdType);
        uint32_t elemTypesSize = pod.elemNumber * sizeof(int);

        uint32_t elemDataSize = 0;
        MeshIterator elemIt(*mesh);
        do
        {
            auto &elem = elemIt.currentElem();
            elemDataSize += static_cast<uint32_t>(getNodeNumber(elem.getType()) * sizeof(IdType));
        } while (elemIt.nextElem());

        pod.nodeIdsOffset = 0;
        pod.nodeDataOffset = nodeIdsSize;
        pod.elemIdsOffset = pod.nodeDataOffset + nodeDataSize;
        pod.elemTypesOffset = pod.elemIdsOffset + elemIdsSize;
        pod.elemDataOffset = pod.elemTypesOffset + elemTypesSize;
        pod.totalDataSize = pod.elemDataOffset + elemDataSize;

        pod.data = new uint8_t[pod.totalDataSize];

        MeshIterator nodeIt(*mesh);
        uint32_t nodeIndex = 0;
        do
        {
            auto &node = nodeIt.currentNode();
            IdType id = node.getId();
            writePrimitive(pod.data, pod.nodeIdsOffset + nodeIndex * sizeof(IdType), &id, sizeof(IdType));

            Scalar xyz[3] = {node.x(), node.y(), node.z()};
            writePrimitive(pod.data, pod.nodeDataOffset + nodeIndex * sizeof(Scalar) * 3, xyz, sizeof(Scalar) * 3);
            ++nodeIndex;
        } while (nodeIt.nextNode());

        MeshIterator elemIt2(*mesh);
        uint32_t elemIndex = 0;
        uint32_t elemCursor = pod.elemDataOffset;
        do
        {
            auto &elem = elemIt2.currentElem();
            IdType elemId = elem.getId();
            writePrimitive(pod.data, pod.elemIdsOffset + elemIndex * sizeof(IdType), &elemId, sizeof(IdType));

            int elemType = static_cast<int>(elem.getType());
            writePrimitive(pod.data, pod.elemTypesOffset + elemIndex * sizeof(int), &elemType, sizeof(int));

            const auto &nodes = elem.getNodes();
            for (IdxType j = 0; j < nodes.size(); ++j)
            {
                IdType nodeId = nodes[j];
                writePrimitive(pod.data, elemCursor + j * sizeof(IdType), &nodeId, sizeof(IdType));
            }

            elemCursor += static_cast<uint32_t>(getNodeNumber(elem.getType()) * sizeof(IdType));
            ++elemIndex;
        } while (elemIt2.nextElem());

        return pod;
    }

    MeshPOD MeshToPOD::convert(const Common::SharedPtr<Mesh> mesh)
    {
        return serialize(mesh);
    }

    Common::SharedPtr<Mesh> MeshToPOD::deserialize(const MeshPOD &pod)
    {
        auto mesh = Common::makeShared<Mesh>();

        for (uint32_t i = 0; i < pod.nodeNumber; ++i)
        {
            IdType nodeId = readPrimitive<IdType>(pod.data, pod.nodeIdsOffset + i * sizeof(IdType));
            Scalar x = readPrimitive<Scalar>(pod.data, pod.nodeDataOffset + i * sizeof(Scalar) * 3 + 0 * sizeof(Scalar));
            Scalar y = readPrimitive<Scalar>(pod.data, pod.nodeDataOffset + i * sizeof(Scalar) * 3 + 1 * sizeof(Scalar));
            Scalar z = readPrimitive<Scalar>(pod.data, pod.nodeDataOffset + i * sizeof(Scalar) * 3 + 2 * sizeof(Scalar));

            Node node;
            node.setId(nodeId);
            CoordType coord(x, y, z);
            node.setXYZ(coord);
            mesh->addNode(Common::move(node));
        }

        uint32_t elemCursor = pod.elemDataOffset;
        for (uint32_t i = 0; i < pod.elemNumber; ++i)
        {
            IdType elemId = readPrimitive<IdType>(pod.data, pod.elemIdsOffset + i * sizeof(IdType));
            int elemTypeInt = readPrimitive<int>(pod.data, pod.elemTypesOffset + i * sizeof(int));
            ElemType elemType = static_cast<ElemType>(elemTypeInt);

            Elem elem;
            elem.setId(elemId);
            elem.setType(elemType);

            const IdxType nodeCount = static_cast<IdxType>(getNodeNumber(elemType));
            for (IdxType j = 0; j < nodeCount; ++j)
            {
                IdType nodeId = readPrimitive<IdType>(pod.data, elemCursor + j * sizeof(IdType));
                elem.setNode(j, nodeId);
            }

            mesh->addElem(Common::move(elem));
            elemCursor += static_cast<uint32_t>(nodeCount * sizeof(IdType));
        }

        return mesh;
    }
} // namespace EgLab::ME