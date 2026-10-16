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
        Common::DynamicArray<IdxType> _boundaryFaceNodeIdx;
        Common::DynamicArray<IdxType> _boundaryLineIdx;
        Common::DynamicArray<ME::ElemFaceLocation> _boundaryFaceLocations;
        Common::DynamicArray<ME::ElemFaceEdgeLocation> _boundaryLineLocations;

        bool _dirty{false};

        void setup()
        {
            _dirty = false;
            _boundaryFaceIdx.clear();
            _boundaryFaceNodeIdx.clear();
            _boundaryLineIdx.clear();
            _boundaryFaceLocations.clear();
            _boundaryLineLocations.clear();

            ME::ExtractSurface extractor(_mesh);
            auto faceIndices = extractor.getSurface();

            _boundaryFaceIdx.reserve(faceIndices.size() * 6);
            _boundaryLineIdx.reserve(faceIndices.size() * 8);

            const IdxType invalidIndex = static_cast<IdxType>(-1);
            Common::DynamicArray<IdxType> faceNodeIndices;
            faceNodeIndices.resize(_mesh->getNodeNumber());
            for (size_t node = 0; node < faceNodeIndices.size(); ++node)
            {
                faceNodeIndices[node] = invalidIndex;
            }

            for (const auto& faceIdxPair : faceIndices)
            {
                auto& elemId = faceIdxPair.first;
                auto& faceIdx = faceIdxPair.second;
                ME::ElemFaceGetter getter(_mesh->getElemById(elemId));
                auto faceNodes = getter.getNodes(faceIdx);
                auto lineNodes = getter.getLineNodes(faceIdx);
                const IdxType triangleCount = static_cast<IdxType>(faceNodes.size() / 3);
                const IdxType edgeCount = static_cast<IdxType>(lineNodes.size() / 2);

                Common::DynamicArray<IdxType> faceMeshNodeIndices;
                faceMeshNodeIndices.reserve(faceNodes.size());
                for (size_t node = 0; node < faceNodes.size(); ++node)
                {
                    IdxType meshNodeIndex = _mesh->getNodeIdx(faceNodes[node]);
                    if (meshNodeIndex >= faceNodeIndices.size())
                    {
                        continue;
                    }

                    IdxType& faceNodeIndex = faceNodeIndices[meshNodeIndex];
                    if (faceNodeIndex == invalidIndex)
                    {
                        faceNodeIndex = static_cast<IdxType>(_boundaryFaceNodeIdx.size());
                        _boundaryFaceNodeIdx.pushBack(meshNodeIndex);
                        faceMeshNodeIndices.pushBack(meshNodeIndex);
                    }
                    _boundaryFaceIdx.pushBack(faceNodeIndex);
                }
                for (IdxType meshNodeIndex : faceMeshNodeIndices)
                {
                    faceNodeIndices[meshNodeIndex] = invalidIndex;
                }
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

    const Common::DynamicArray<IdxType>& MeshToTriFace::getBoundaryFaceNodeIdx() const
    {
        if (_impl->_dirty) _impl->setup();

        return _impl->_boundaryFaceNodeIdx;
    }

    Common::DynamicArray<IdxType> MeshToTriFace::getBoundaryLineIdx() const
    {
        if (_impl->_dirty) _impl->setup();

        return _impl->_boundaryLineIdx;
    }

    const Common::DynamicArray<ME::ElemFaceLocation>& MeshToTriFace::getBoundaryFaceLocations()
        const
    {
        if (_impl->_dirty) _impl->setup();

        return _impl->_boundaryFaceLocations;
    }

    const Common::DynamicArray<ME::ElemFaceEdgeLocation>& MeshToTriFace::getBoundaryLineLocations()
        const
    {
        if (_impl->_dirty) _impl->setup();

        return _impl->_boundaryLineLocations;
    }

} // namespace EgLab::RE