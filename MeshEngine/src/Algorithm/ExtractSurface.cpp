#include "MeshEngine/Algorithm/ExtractSurface.hpp"

#include "Common/HashSet.hpp"
#include "Common/Loop.hpp"
#include "Common/Sort.hpp"

namespace EgLab::ME
{

    /**
     * @brief simple element face, three node represent one face.
     *        so it's not support strength faces
     *
     */
    struct ElemFace
    {
        void setNodes(IdType n1, IdType n2, IdType n3)
        {
            Common::sort3(n1, n2, n3);
            _n1 = n1;
            _n2 = n2;
            _n3 = n3;
        }

        void setNodes(IdType n1, IdType n2, IdType n3, IdType n4)
        {
            Common::sort3(n1, n2, n3);
            _n1 = n1;
            _n2 = n2;
            _n3 = n3;
            if (n4 < n3)
            {
                if (n4 < n2)
                {
                    if (n4 < n1)
                    {
                        _n3 = _n2;
                        _n2 = _n1;
                        _n1 = n4;
                    }
                    else
                    {
                        _n3 = _n2;
                        _n2 = n4;
                    }
                }
                else
                {
                    _n3 = n4;
                }
            }
        }

        ElemFace(const Elem& e, IdxType faceIdx)
        {
            ElemFaceGetter getter(e);
            int nodeNumber = getter.elemFaceNodeNumber(faceIdx);
            // A face is identified by its corner nodes only. Quadratic faces (6 or 8 nodes)
            // must ignore their midside nodes so that the two elements sharing a face still
            // produce the same key.
            if (nodeNumber == 3 || nodeNumber == 6)
            {
                setNodes(getter.elemFace(faceIdx, 0), getter.elemFace(faceIdx, 1),
                         getter.elemFace(faceIdx, 2));
            }
            else if (nodeNumber == 4 || nodeNumber == 8)
            {
                setNodes(getter.elemFace(faceIdx, 0), getter.elemFace(faceIdx, 1),
                         getter.elemFace(faceIdx, 2), getter.elemFace(faceIdx, 3));
            }
            else
            {
                // TODO:ERROR
            }

            _elemId = e.getId();
            _faceIdx = faceIdx;
        }

        bool operator<=(ElemFace& other) const
        {
            if (_n1 != other._n1) return _n1 < other._n1;
            if (_n2 != other._n2) return _n2 < other._n2;
            if (_n3 != other._n3) return _n3 < other._n3;
            return true;
        }

        bool operator==(ElemFace& other) const
        {
            return _n1 == other._n1 && _n2 == other._n2 && _n3 == other._n3;
        }

        bool operator!=(ElemFace& other) const
        {
            return _n1 != other._n1 || _n2 != other._n2 || _n3 != other._n3;
        }

        IdType _n1{0};
        IdType _n2{0};
        IdType _n3{0};

        IdType _elemId{0};
        IdxType _faceIdx{0};
    };

    class ExtractSurface::Impl
    {
    public:
        Common::SharedPtr<Mesh> _mesh;
        Common::HashSet<IdType> _hiddenElementIds;
    };

    ExtractSurface::ExtractSurface(Common::SharedPtr<Mesh> mesh) : _impl(Common::makeUnique<Impl>())
    {
        _impl->_mesh = mesh;
    }

    ExtractSurface::~ExtractSurface()
    {
    }

    void ExtractSurface::setHiddenElementIds(
        const Common::DynamicArray<IdType>& hiddenElementIds)
    {
        _impl->_hiddenElementIds.clear();
        for (size_t index = 0; index < hiddenElementIds.size(); ++index)
        {
            _impl->_hiddenElementIds.insert(hiddenElementIds[index]);
        }
    }

    Common::DynamicArray<Common::Pair<IdType, IdxType>> ExtractSurface::getSurface()
    {
        if (_impl->_mesh == nullptr) return {};

        const bool filterHidden = !_impl->_hiddenElementIds.empty();

        Common::DynamicArray<Common::Pair<IdType, IdxType>> idxs;
        Common::DynamicArray<ElemFace> elemFaces;

        MeshIterator it(*_impl->_mesh);

        for (; it.hasMoreElem(); it.nextElem())
        {
            auto& e = it.currentElem();
            if (filterHidden &&
                _impl->_hiddenElementIds.find(e.getId()) != _impl->_hiddenElementIds.end())
            {
                continue;
            }

            ElemFaceGetter getter(e);

            for (int i = 0; i < getter.elemFaceNumber(); i++)
            {
                ElemFace ef(e, i);
                elemFaces.pushBack(Common::move(ef));
            }
        }

        Common::sort(elemFaces);

        for (int i = 0; i < elemFaces.size();)
        {
            int faceCount = 0;
            int j = i + 1;
            for (; j < elemFaces.size(); j++)
            {
                if (elemFaces[i] != elemFaces[j]) break;
                faceCount++;
            }
            if (faceCount == 0)
            {
                ElemFaceGetter getter(_impl->_mesh->getElemById(elemFaces[i]._elemId));
                idxs.pushBack({elemFaces[i]._elemId, elemFaces[i]._faceIdx});
            }
            i = j;
        }

        return idxs;
    }
} // namespace EgLab::ME