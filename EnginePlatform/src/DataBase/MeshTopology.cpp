#include "DataBase/MeshTopology.hpp"

#include "MeshEngine/MeshData/Element.hpp"
#include "MeshEngine/MeshData/Node.hpp"

namespace EgLab::Platform
{
    namespace
    {
        constexpr int32_t ALL_ELEMENT_TYPES = -1;
    } // namespace

    MeshTopology::MeshTopology(const Common::SharedPtr<ME::Mesh>& mesh) : _mesh(mesh)
    {
        if (!_mesh)
        {
            return;
        }

        ME::MeshIterator nodeIterator(*_mesh);
        for (; nodeIterator.hasMoreNode(); nodeIterator.nextNode())
        {
            ++_nodeCount;
        }

        ME::MeshIterator elemIterator(*_mesh);
        for (; elemIterator.hasMoreElem(); elemIterator.nextElem())
        {
            ++_elementCount;

            const int32_t type = static_cast<int32_t>(elemIterator.currentElem().getType());
            bool found = false;
            for (auto& entry : _elementTypeCounts)
            {
                if (entry.type == type)
                {
                    ++entry.count;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                MeshElementTypeCount entry;
                entry.type = type;
                entry.count = 1;
                _elementTypeCounts.pushBack(entry);
            }
        }
    }

    void MeshTopology::collectNodes(Common::DynamicArray<MeshNodeRecord>& out) const
    {
        out.clear();
        if (!_mesh)
        {
            return;
        }

        out.reserve(_nodeCount);
        ME::MeshIterator nodeIterator(*_mesh);
        for (; nodeIterator.hasMoreNode(); nodeIterator.nextNode())
        {
            out.pushBack(makeNodeRecord(nodeIterator.currentNode()));
        }
    }

    void MeshTopology::collectElements(int32_t elementType,
                                       Common::DynamicArray<MeshElementRecord>& out) const
    {
        out.clear();
        if (!_mesh)
        {
            return;
        }

        if (elementType == ALL_ELEMENT_TYPES)
        {
            out.reserve(_elementCount);
        }

        ME::MeshIterator elemIterator(*_mesh);
        for (; elemIterator.hasMoreElem(); elemIterator.nextElem())
        {
            const ME::Elem& elem = elemIterator.currentElem();
            if (elementType != ALL_ELEMENT_TYPES &&
                static_cast<int32_t>(elem.getType()) != elementType)
            {
                continue;
            }
            out.pushBack(makeElementRecord(elem));
        }
    }

    void MeshTopology::collectNodeElements(uint32_t nodeId,
                                           Common::DynamicArray<MeshElementRecord>& out) const
    {
        out.clear();
        if (!_mesh || _mesh->getNodeIdx(nodeId) == ME::INVALID_IDX)
        {
            return;
        }

        ME::MeshIterator elemIterator(*_mesh);
        for (; elemIterator.hasMoreElem(); elemIterator.nextElem())
        {
            const ME::Elem& elem = elemIterator.currentElem();
            const auto& nodeIds = elem.getNodes();
            bool referencesNode = false;
            for (std::size_t index = 0; index < nodeIds.size(); ++index)
            {
                if (nodeIds[index] == nodeId)
                {
                    referencesNode = true;
                    break;
                }
            }
            if (referencesNode)
            {
                out.pushBack(makeElementRecord(elem));
            }
        }
    }

    void MeshTopology::collectElementNodes(uint32_t elementId,
                                           Common::DynamicArray<MeshNodeRecord>& out) const
    {
        out.clear();
        if (!_mesh || _mesh->getElemIdx(elementId) == ME::INVALID_IDX)
        {
            return;
        }

        const ME::Elem& elem = _mesh->getElemById(elementId);
        const auto& nodeIds = elem.getNodes();
        out.reserve(nodeIds.size());
        for (std::size_t index = 0; index < nodeIds.size(); ++index)
        {
            const ME::IdType nodeId = nodeIds[index];
            if (_mesh->getNodeIdx(nodeId) == ME::INVALID_IDX)
            {
                continue;
            }
            out.pushBack(makeNodeRecord(_mesh->getNodeById(nodeId)));
        }
    }

    MeshElementRecord MeshTopology::makeElementRecord(const ME::Elem& elem)
    {
        MeshElementRecord record;
        record.id = elem.getId();
        record.type = static_cast<int32_t>(elem.getType());
        return record;
    }

    MeshNodeRecord MeshTopology::makeNodeRecord(ME::Node& node)
    {
        MeshNodeRecord record;
        record.id = node.getId();
        record.x = static_cast<float>(node.x());
        record.y = static_cast<float>(node.y());
        record.z = static_cast<float>(node.z());
        return record;
    }

    const char* MeshTopology::elementTypeName(int32_t elementType)
    {
        switch (static_cast<ME::ElemType>(elementType))
        {
        case ME::ElemType::Tri3:
            return "Tri3";
        case ME::ElemType::Tri6:
            return "Tri6";
        case ME::ElemType::Quad4:
            return "Quad4";
        case ME::ElemType::Quad8:
            return "Quad8";
        case ME::ElemType::Tet4:
            return "Tet4";
        case ME::ElemType::Tet10:
            return "Tet10";
        case ME::ElemType::Wed6:
            return "Wed6";
        case ME::ElemType::Prd5:
            return "Prd5";
        case ME::ElemType::Hex8:
            return "Hex8";
        case ME::ElemType::Hex20:
            return "Hex20";
        default:
            return "Unknown";
        }
    }
} // namespace EgLab::Platform
