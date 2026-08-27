#include "RenderEngine/Core/MeshToTriFace.hpp"

#include "Common/DynamicArray.hpp"
#include "MeshEngine/Algorithm/ExtractSurface.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab::RE
{
    class MeshToTriFace::Impl
    {
    public:
        Common::DynamicArray<Common::SharedPtr<ME::Mesh>> _meshes;
        Common::SharedPtr<ME::Mesh> _mesh;
        Common::DynamicArray<IdxType> _boundaryFaceIdx;
        Common::DynamicArray<IdxType> _boundaryLineIdx;

        bool _dirty{false};

        void setup()
        {
            _dirty = false;
            ME::ExtractSurface extractor(_mesh);
            auto faceIndices = extractor.getSurface();
            for (const auto& faceIdxPair : faceIndices)
            {
                auto& elemId = faceIdxPair.first;
                auto& faceIdx = faceIdxPair.second;
                ME::ElemFaceGetter getter(_mesh->getElemById(elemId));
                _boundaryFaceIdx.pushBack(getter.getNodes(faceIdx));
                _boundaryLineIdx.pushBack(getter.getLineNodes(faceIdx));
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
        //_impl->_meshes.pushBack(mesh);
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

} // namespace EgLab::RE