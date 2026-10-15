#include "RenderEngine/Core/MeshToTriFace.hpp"

#include "Common/DynamicArray.hpp"
#include "MeshEngine/Algorithm/ExtractSurface.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab::RE
{
    class MeshToTriFace::Impl
    {
    public:
        Common::SharedPtr<ME::Mesh> _mesh;
        Common::DynamicArray<IdxType> _boundaryFaceIdx;
        Common::DynamicArray<IdxType> _boundaryLineIdx;
        Common::DynamicArray<ME::ElemFaceLocation> _boundaryFaceLocations;
        Common::DynamicArray<ME::ElemFaceEdgeLocation> _boundaryLineLocations;

        bool _dirty{false};

        void setup()
        {
            _dirty = false;
            _boundaryFaceIdx.clear();
            _boundaryLineIdx.clear();
            _boundaryFaceLocations.clear();
            _boundaryLineLocations.clear();

            ME::ExtractSurface extractor(_mesh);
            auto faceIndices = extractor.getSurface();

            _boundaryFaceIdx.reserve(faceIndices.size() * 6);
            _boundaryLineIdx.reserve(faceIndices.size() * 8);

            for (const auto& faceIdxPair : faceIndices)
            {
                auto& elemId = faceIdxPair.first;
                auto& faceIdx = faceIdxPair.second;
                ME::ElemFaceGetter getter(_mesh->getElemById(elemId));
                auto faceNodes = getter.getNodes(faceIdx);
                auto lineNodes = getter.getLineNodes(faceIdx);
                const IdxType triangleCount = static_cast<IdxType>(faceNodes.size() / 3);
                const IdxType edgeCount = static_cast<IdxType>(lineNodes.size() / 2);

                _boundaryFaceIdx.pushBack(Common::move(faceNodes));
                _boundaryLineIdx.pushBack(Common::move(lineNodes));
                for (IdxType triangle = 0; triangle < triangleCount; ++triangle)
                {
                    _boundaryFaceLocations.pushBack({elemId, faceIdx});
                }
                for (IdxType edge = 0; edge < edgeCount; ++edge)
                {
                    _boundaryLineLocations.pushBack({elemId, faceIdx, edge});
                }
            }

            for (int i = 0; i < _boundaryFaceIdx.size(); i++)
            {
                _boundaryFaceIdx[i] = _mesh->getNodeIdx(_boundaryFaceIdx[i]);
            }

            for (int i = 0; i < _boundaryLineIdx.size(); i++)
            {
                _boundaryLineIdx[i] = _mesh->getNodeIdx(_boundaryLineIdx[i]);
            }
        }
    };

    MeshToTriFace::MeshToTriFace() : _impl(Common::makeUnique<Impl>())
    {
    }

    MeshToTriFace::~MeshToTriFace()
    {
    }

    void MeshToTriFace::setMesh(Common::SharedPtr<ME::Mesh>& mesh)
    {
        _impl->_mesh = mesh;
        _impl->_dirty = true;
    }

    Common::DynamicArray<IdxType> MeshToTriFace::getBoundaryFaceIdx() const
    {
        if (_impl->_dirty) _impl->setup();

        return _impl->_boundaryFaceIdx;
    }

    Common::DynamicArray<IdxType> MeshToTriFace::getBoundaryLineIdx() const
    {
        if (_impl->_dirty) _impl->setup();

        return _impl->_boundaryLineIdx;
    }

    Common::DynamicArray<ME::ElemFaceLocation> MeshToTriFace::getBoundaryFaceLocations() const
    {
        if (_impl->_dirty) _impl->setup();

        return _impl->_boundaryFaceLocations;
    }

    Common::DynamicArray<ME::ElemFaceEdgeLocation> MeshToTriFace::getBoundaryLineLocations() const
    {
        if (_impl->_dirty) _impl->setup();

        return _impl->_boundaryLineLocations;
    }

} // namespace EgLab::RE