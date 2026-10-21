#include "MeshEngine/MeshData/HEMesh.hpp"

#include "Common/DynamicArray.hpp"
#include "Common/HashMap.hpp"
#include "Common/HashSet.hpp"
#include "MeshEngine/Common/SoaContainer.hpp"
#include "MeshEngine/MeshData/HEFace.hpp"
#include "MeshEngine/MeshData/HENode.hpp"
#include "MeshEngine/MeshData/HalfEdge.hpp"

namespace EgLab::ME
{

    class HEMesh::Impl
    {
    public:
        HalfEdges _halfEdges;
        HEFaces _faces;
        HENodes _vertices;
        Common::HashMap<IdType, IdxType> _vertexIdToIdx;
    };

    HEMesh::HEMesh() : _impl(Common::makeUnique<Impl>())
    {
    }

    HEMesh::~HEMesh()
    {
    }

    IdType HEMesh::createVertex(float x, float y, float z)
    {
        const auto index = static_cast<IdxType>(_impl->_vertices.size());
        auto id = static_cast<IdType>(index + 1);
        while (_impl->_vertexIdToIdx.find(id) != _impl->_vertexIdToIdx.end())
        {
            ++id;
        }
        createVertex(id, x, y, z);
        return id;
    }

    Common::Return HEMesh::createVertex(IdType id, float x, float y, float z)
    {
        if (id == INVALID_IDX || _impl->_vertexIdToIdx.find(id) != _impl->_vertexIdToIdx.end())
        {
            return Common::Return::BadInput;
        }

        const auto index = static_cast<IdxType>(_impl->_vertices.size());
        HENode vertex{id, CoordType(x, y, z), INVALID_IDX};
        _impl->_vertices.pushBack(vertex);
        _impl->_vertexIdToIdx.insert({id, index});
        return Common::Return::Succeed;
    }

    Common::Return HEMesh::createFace(const Common::DynamicArray<IdType>& vertexIds)
    {
        if (vertexIds.size() < 3)
        {
            return Common::Return::BadInput;
        }

        Common::DynamicArray<IdxType> vertexIndices;
        vertexIndices.reserve(vertexIds.size());
        Common::HashSet<IdxType> uniqueVertices;
        for (std::size_t i = 0; i < vertexIds.size(); ++i)
        {
            const auto found = _impl->_vertexIdToIdx.find(vertexIds[i]);
            if (found == _impl->_vertexIdToIdx.end() || !uniqueVertices.insert((*found).second))
            {
                return Common::Return::BadInput;
            }
            auto vertexIndex = (*found).second;
            vertexIndices.pushBack(vertexIndex);
        }

        Common::DynamicArray<IdxType> twinIndices;
        twinIndices.reserve(vertexIndices.size());
        auto invalidIndex = INVALID_IDX;
        for (std::size_t edge = 0; edge < vertexIndices.size(); ++edge)
        {
            twinIndices.pushBack(invalidIndex);
        }
        for (std::size_t edge = 0; edge < vertexIndices.size(); ++edge)
        {
            const auto from = vertexIndices[edge];
            const auto to = vertexIndices[(edge + 1) % vertexIndices.size()];
            for (IdxType existing = 0; existing < _impl->_halfEdges.size(); ++existing)
            {
                const auto existingFrom = _impl->_halfEdges.get(existing, &HalfEdge::vertex);
                const auto existingNext = _impl->_halfEdges.get(existing, &HalfEdge::next);
                const auto existingTo = _impl->_halfEdges.get(existingNext, &HalfEdge::vertex);
                if (existingFrom == from && existingTo == to)
                {
                    return Common::Return::BadInput;
                }
                if (existingFrom == to && existingTo == from)
                {
                    if (_impl->_halfEdges.get(existing, &HalfEdge::twin) != INVALID_IDX)
                    {
                        return Common::Return::BadInput;
                    }
                    twinIndices[edge] = existing;
                }
            }
        }

        const auto faceIndex = static_cast<IdxType>(_impl->_faces.size());
        const auto firstHalfEdge = static_cast<IdxType>(_impl->_halfEdges.size());
        HEFace face{static_cast<IdType>(faceIndex + 1), firstHalfEdge};
        _impl->_faces.pushBack(face);

        for (std::size_t edge = 0; edge < vertexIndices.size(); ++edge)
        {
            const auto halfEdgeIndex = firstHalfEdge + static_cast<IdxType>(edge);
            HalfEdge halfEdge{
                faceIndex, vertexIndices[edge],
                firstHalfEdge + static_cast<IdxType>((edge + 1) % vertexIndices.size()),
                firstHalfEdge +
                    static_cast<IdxType>((edge + vertexIndices.size() - 1) % vertexIndices.size()),
                twinIndices[edge]};
            _impl->_halfEdges.pushBack(halfEdge);
            if (_impl->_vertices.get(halfEdge.vertex, &HENode::_halfEdge) == INVALID_IDX)
            {
                _impl->_vertices.set(halfEdge.vertex, &HENode::_halfEdge, halfEdgeIndex);
            }
            if (twinIndices[edge] != INVALID_IDX)
            {
                _impl->_halfEdges.set(twinIndices[edge], &HalfEdge::twin, halfEdgeIndex);
            }
        }

        return Common::Return::Succeed;
    }

    std::size_t HEMesh::getVertexCount() const
    {
        return _impl->_vertices.size();
    }

    std::size_t HEMesh::getFaceCount() const
    {
        return _impl->_faces.size();
    }

    std::size_t HEMesh::getHalfEdgeCount() const
    {
        return _impl->_halfEdges.size();
    }

    HENode HEMesh::getVertex(IdxType index) const
    {
        return HENode{_impl->_vertices.get(index, &HENode::_id),
                      _impl->_vertices.get(index, &HENode::_coord),
                      _impl->_vertices.get(index, &HENode::_halfEdge)};
    }

    IdxType HEMesh::getVertexIndex(IdType id) const
    {
        const auto found = _impl->_vertexIdToIdx.find(id);
        return found == _impl->_vertexIdToIdx.end() ? INVALID_IDX : (*found).second;
    }

    HEFace HEMesh::getFace(IdxType index) const
    {
        return HEFace{_impl->_faces.get(index, &HEFace::_id),
                      _impl->_faces.get(index, &HEFace::_halfEdge)};
    }

    HalfEdge HEMesh::getHalfEdge(IdxType index) const
    {
        return HalfEdge{_impl->_halfEdges.get(index, &HalfEdge::face),
                        _impl->_halfEdges.get(index, &HalfEdge::vertex),
                        _impl->_halfEdges.get(index, &HalfEdge::next),
                        _impl->_halfEdges.get(index, &HalfEdge::prev),
                        _impl->_halfEdges.get(index, &HalfEdge::twin)};
    }
} // namespace EgLab::ME
