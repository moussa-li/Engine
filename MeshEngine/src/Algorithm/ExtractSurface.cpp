#include "MeshEngine/Algorithm/ExtractSurface.hpp"

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
            if (nodeNumber == 3)
            {
                setNodes(getter.elemFace(faceIdx, 0), getter.elemFace(faceIdx, 1),
                         getter.elemFace(faceIdx, 2));
            }
            else if (nodeNumber == 4)
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

        IdType _n1;
        IdType _n2;
        IdType _n3;

        IdType _elemId;
        IdxType _faceIdx;
    };

    class ExtractSurface::Impl
    {
    public:
        Common::SharedPtr<Mesh> _mesh;
    };

    ExtractSurface::ExtractSurface(Common::SharedPtr<Mesh> mesh) : _impl(Common::makeUnique<Impl>())
    {
        _impl->_mesh = mesh;
    }

    ExtractSurface::~ExtractSurface()
    {
    }

    Common::DynamicArray<Common::Pair<IdType, IdxType>> ExtractSurface::getSurface()
    {
        if (_impl->_mesh == nullptr) return {};

        Common::DynamicArray<Common::Pair<IdType, IdxType>> idxs;
        Common::DynamicArray<ElemFace> elemFaces;

        MeshIterator it(*_impl->_mesh);

        do
        {
            auto& e = it.currentElem();
            ElemFaceGetter getter(e);

            for (int i = 0; i < getter.elemFaceNumber(); i++)
            {
                ElemFace ef(e, i);
                elemFaces.pushBack(Common::move(ef));
            }

        } while (it.nextElem());

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