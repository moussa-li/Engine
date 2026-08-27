#include "MeshEngine/MeshData/Element.hpp"

namespace EgLab::ME
{
    /*********************** Tri4 ****************************/
    int tri3FaceNodeNumber[1] = {3};

    int tri3ElemFaceOffset[1] = {0};

    IdType tri3elemFaceMap[1][3] = {
        {0, 1, 2},
    };

    /*********************** Tet4 ****************************/

    int tet4FaceNodeNumber[4] = {3, 3, 3, 3};

    int tet4ElemFaceOffset[4] = {0, 3, 6, 9};

    IdType tet4elemFaceMap[4][3] = {
        {0, 1, 2},
        {0, 2, 3},
        {0, 3, 1},
        {1, 3, 2},
    };

    int getTypeDim(ElemType type)
    {
        return (int)type >> 8 & 0x0F;
    }

    ElemFaceIterator::ElemFaceIterator(const Elem &elem) : _elem(elem)
    {
    }

    ElemFaceGetter::ElemFaceGetter(const Elem &elem) : _elem(elem)
    {
        switch (_elem.getType())
        {
        case ElemType::Tri3:
        {
            _elemFaceNodeNumberMap = tri3FaceNodeNumber;
            _elemFaceOffset = tri3ElemFaceOffset;
            _elemFaceMap = tri3elemFaceMap[0];
            break;
        }
        case ElemType::Tet4:
        {
            _elemFaceNodeNumberMap = tet4FaceNodeNumber;
            _elemFaceOffset = tet4ElemFaceOffset;
            _elemFaceMap = tet4elemFaceMap[0];
            break;
        }
        case ElemType::Tri6:
        case ElemType::Quad4:
        case ElemType::Quad8:
        case ElemType::Tet10:
        case ElemType::Wed6:
        case ElemType::Prd5:
        case ElemType::Hex8:
        case ElemType::Hex20:
        default:
            break;
        }
    }

    int ElemFaceGetter::elemFaceNumber() const
    {
        if (getTypeDim(_elem.getType()) <= 2) return 1;

        switch (_elem.getType())
        {
        case ElemType::Tet4:
        case ElemType::Tet10:
            return 4;
        case ElemType::Wed6:
        case ElemType::Prd5:
            return 5;
        case ElemType::Hex8:
        case ElemType::Hex20:
            return 6;
        default:
            return 0;
        }
        return 0;
    }

    int ElemFaceGetter::elemFaceNodeNumber(IdxType elemFaceIdx) const
    {
        // TODO : add debug check
        return _elemFaceNodeNumberMap[elemFaceIdx];
    }

    IdType ElemFaceGetter::elemFace(IdxType elemFaceIdx, IdxType nodeIdx) const
    {
        IdxType node = _elemFaceMap[_elemFaceOffset[elemFaceIdx] + nodeIdx];
        return _elem.getNodes()[node];
    }

    Common::DynamicArray<IdType> ElemFaceGetter::getNodes(IdxType elemFaceIdx) const
    {
        Common::DynamicArray<IdType> res;
        IdxType nodeNumber = elemFaceNodeNumber(elemFaceIdx);
        if (nodeNumber == 3)
        {
            res.resize(nodeNumber);
            for (int i = 0; i < nodeNumber; i++)
            {
                res[i] = _elemFaceMap[_elemFaceOffset[elemFaceIdx] + i];
            }
        }
        else if (nodeNumber == 4)
        {
            static int quadFaceMap[6] = {0, 1, 2, 1, 3, 2};
            res.resize(6);
            for (int i = 0; i < 6; i++)
            {
                res[i] = _elemFaceMap[_elemFaceOffset[elemFaceIdx] + quadFaceMap[i]];
            }
        }
        else
        {
        }
        return res;
    }

    Common::DynamicArray<IdType> ElemFaceGetter::getLineNodes(IdxType elemFaceIdx) const
    {
        Common::DynamicArray<IdType> res;
        IdxType nodeNumber = elemFaceNodeNumber(elemFaceIdx);
        if (nodeNumber == 3)
        {
            static int triLineMap[6] = {0, 1, 1, 2, 2, 0};
            res.resize(6);
            for (int i = 0; i < 6; i++)
            {
                res[i] = _elemFaceMap[_elemFaceOffset[elemFaceIdx] + triLineMap[i]];
            }
        }
        else if (nodeNumber == 4)
        {
            static int quadLineMap[8] = {0, 1, 1, 2, 2, 3, 3, 0};
            res.resize(8);
            for (int i = 0; i < 8; i++)
            {
                res[i] = _elemFaceMap[_elemFaceOffset[elemFaceIdx] + quadLineMap[i]];
            }
        }
        else
        {
        }
        return res;
    }

} // namespace EgLab::ME
