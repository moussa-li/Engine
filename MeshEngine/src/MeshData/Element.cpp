#include "MeshEngine/MeshData/Element.hpp"

namespace EgLab::ME
{
    /*********************** Tri3 ****************************/
    int tri3FaceNodeNumber[1] = {3};

    int tri3ElemFaceOffset[1] = {0};

    IdType tri3elemFaceMap[1][3] = {
        {0, 1, 2},
    };

    /*********************** Quad4 ****************************/
    int quad4FaceNodeNumber[1] = {4};

    int quad4ElemFaceOffset[1] = {0};

    IdType quad4elemFaceMap[1][4] = {
        {0, 1, 2, 3},
        // {2, 1, 3},
    };

    int tri6FaceNodeNumber[1] = {6};

    int tri6ElemFaceOffset[1] = {0};

    IdType tri6elemFaceMap[1][6] = {
        {0, 1, 2, 3, 4, 5},
    };

    int quad8FaceNodeNumber[1] = {8};

    int quad8ElemFaceOffset[1] = {0};

    IdType quad8elemFaceMap[1][8] = {
        {0, 1, 2, 3, 4, 5, 6, 7},
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

    int tet10FaceNodeNumber[4] = {6, 6, 6, 6};

    int tet10ElemFaceOffset[4] = {0, 6, 12, 18};

    IdType tet10elemFaceMap[4][6] = {
        {0, 1, 2, 4, 5, 6},
        {0, 2, 3, 6, 9, 7},
        {0, 3, 1, 7, 8, 4},
        {1, 3, 2, 8, 9, 5},
    };

    /*********************** Wed6 ****************************/

    int wed6FaceNodeNumber[5] = {3, 3, 4, 4, 4};

    int wed6ElemFaceOffset[5] = {0, 3, 6, 10, 14};

    IdType wed6elemFaceMap[17] = {
        0, 2, 1, 3, 4, 5, 0, 1, 4, 3, 1, 2, 5, 4, 2, 0, 3,
    };

    /*********************** Prd5 ****************************/

    int prd5FaceNodeNumber[5] = {4, 3, 3, 3, 3};

    int prd5ElemFaceOffset[5] = {0, 4, 7, 10, 13};

    IdType prd5elemFaceMap[16] = {
        0, 1, 2, 3, 0, 1, 4, 1, 2, 4, 2, 3, 4, 3, 0, 4,
    };

    /*********************** Hex8 ****************************/

    int hex8FaceNodeNumber[6] = {4, 4, 4, 4, 4, 4};

    int hex8ElemFaceOffset[6] = {0, 4, 8, 12, 16, 20};

    IdType hex8elemFaceMap[24] = {
        0, 1, 2, 3, 4, 7, 6, 5, 0, 1, 5, 4, 1, 2, 6, 5, 2, 3, 7, 6, 3, 0, 4, 7,
    };

    /*********************** Hex20 ****************************/

    int hex20FaceNodeNumber[6] = {8, 8, 8, 8, 8, 8};

    int hex20ElemFaceOffset[6] = {0, 8, 16, 24, 32, 40};

    IdType hex20elemFaceMap[48] = {
        0, 1, 2, 3, 8, 9,  10, 11, 4, 7, 6, 5, 15, 14, 13, 12, 0, 1, 5, 4, 8,  17, 12, 16,
        1, 2, 6, 5, 9, 18, 13, 17, 2, 3, 7, 6, 10, 19, 14, 18, 3, 0, 4, 7, 11, 16, 15, 19,
    };

    int getTypeDim(ElemType type)
    {
        return (int)type >> 8 & 0x0F;
    }

    int getNodeNumber(ElemType type)
    {
        return (int)type & 0x0F;
    }

    ElemFaceIterator::ElemFaceIterator(const Elem &elem) : _elem(elem)
    {
    }

    ElemFaceGetter::ElemFaceGetter(const Elem &elem)
        : _elem(elem),
          _elemFaceNodeNumberMap(nullptr),
          _elemFaceOffset(nullptr),
          _elemFaceMap(nullptr)
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
        case ElemType::Quad4:
        {
            _elemFaceNodeNumberMap = quad4FaceNodeNumber;
            _elemFaceOffset = quad4ElemFaceOffset;
            _elemFaceMap = quad4elemFaceMap[0];
            break;
        }
        case ElemType::Tri6:
        {
            _elemFaceNodeNumberMap = tri6FaceNodeNumber;
            _elemFaceOffset = tri6ElemFaceOffset;
            _elemFaceMap = tri6elemFaceMap[0];
            break;
        }
        case ElemType::Quad8:
        {
            _elemFaceNodeNumberMap = quad8FaceNodeNumber;
            _elemFaceOffset = quad8ElemFaceOffset;
            _elemFaceMap = quad8elemFaceMap[0];
            break;
        }
        case ElemType::Tet4:
        {
            _elemFaceNodeNumberMap = tet4FaceNodeNumber;
            _elemFaceOffset = tet4ElemFaceOffset;
            _elemFaceMap = tet4elemFaceMap[0];
            break;
        }
        case ElemType::Tet10:
        {
            _elemFaceNodeNumberMap = tet10FaceNodeNumber;
            _elemFaceOffset = tet10ElemFaceOffset;
            _elemFaceMap = tet10elemFaceMap[0];
            break;
        }
        case ElemType::Wed6:
        {
            _elemFaceNodeNumberMap = wed6FaceNodeNumber;
            _elemFaceOffset = wed6ElemFaceOffset;
            _elemFaceMap = wed6elemFaceMap;
            break;
        }
        case ElemType::Prd5:
        {
            _elemFaceNodeNumberMap = prd5FaceNodeNumber;
            _elemFaceOffset = prd5ElemFaceOffset;
            _elemFaceMap = prd5elemFaceMap;
            break;
        }
        case ElemType::Hex8:
        {
            _elemFaceNodeNumberMap = hex8FaceNodeNumber;
            _elemFaceOffset = hex8ElemFaceOffset;
            _elemFaceMap = hex8elemFaceMap;
            break;
        }
        case ElemType::Hex20:
        {
            _elemFaceNodeNumberMap = hex20FaceNodeNumber;
            _elemFaceOffset = hex20ElemFaceOffset;
            _elemFaceMap = hex20elemFaceMap;
            break;
        }
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
        if (_elemFaceNodeNumberMap == nullptr || elemFaceIdx >= elemFaceNumber()) return 0;
        return _elemFaceNodeNumberMap[elemFaceIdx];
    }

    IdType ElemFaceGetter::elemFace(IdxType elemFaceIdx, IdxType nodeIdx) const
    {
        if (_elemFaceMap == nullptr || nodeIdx >= elemFaceNodeNumber(elemFaceIdx))
            return INVALID_IDX;
        IdxType node = _elemFaceMap[_elemFaceOffset[elemFaceIdx] + nodeIdx];
        return _elem.getNodes()[node];
    }

    Common::DynamicArray<IdType> ElemFaceGetter::getNodes(IdxType elemFaceIdx) const
    {
        Common::DynamicArray<IdType> res;
        IdxType nodeNumber = elemFaceNodeNumber(elemFaceIdx);
        if (nodeNumber == 3)
        {
            res.resize(3);
            for (int i = 0; i < 3; ++i)
            {
                auto node = _elemFaceMap[_elemFaceOffset[elemFaceIdx] + i];
                res[i] = _elem.getNodes()[node];
            }
        }
        else if (nodeNumber == 6)
        {
            static const int tri6TriMap[12] = {0, 3, 1, 0, 1, 4, 0, 4, 2, 0, 2, 5};
            res.resize(12);
            for (int i = 0; i < 12; ++i)
            {
                auto node = _elemFaceMap[_elemFaceOffset[elemFaceIdx] + tri6TriMap[i]];
                res[i] = _elem.getNodes()[node];
            }
        }
        else if (nodeNumber == 4)
        {
            static int quadFaceMap[6] = {0, 1, 2, 0, 2, 3};
            res.resize(6);
            for (int i = 0; i < 6; i++)
            {
                auto node = _elemFaceMap[_elemFaceOffset[elemFaceIdx] + quadFaceMap[i]];
                res[i] = _elem.getNodes()[node];
            }
        }
        else if (nodeNumber == 8)
        {
            static const int quad8TriMap[18] = {
                0, 4, 1, 0, 1, 5, 0, 5, 2, 0, 2, 6, 0, 6, 3, 0, 3, 7,
            };
            res.resize(18);
            for (int i = 0; i < 18; i++)
            {
                auto node = _elemFaceMap[_elemFaceOffset[elemFaceIdx] + quad8TriMap[i]];
                res[i] = _elem.getNodes()[node];
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
                auto node = _elemFaceMap[_elemFaceOffset[elemFaceIdx] + triLineMap[i]];
                res[i] = _elem.getNodes()[node];
            }
        }
        else if (nodeNumber == 6)
        {
            static const int tri6LineMap[12] = {0, 3, 3, 1, 1, 4, 4, 2, 2, 5, 5, 0};
            res.resize(12);
            for (int i = 0; i < 12; i++)
            {
                auto node = _elemFaceMap[_elemFaceOffset[elemFaceIdx] + tri6LineMap[i]];
                res[i] = _elem.getNodes()[node];
            }
        }
        else if (nodeNumber == 4)
        {
            static int quadLineMap[8] = {0, 1, 1, 2, 2, 3, 3, 0};
            res.resize(8);
            for (int i = 0; i < 8; i++)
            {
                auto node = _elemFaceMap[_elemFaceOffset[elemFaceIdx] + quadLineMap[i]];
                res[i] = _elem.getNodes()[node];
            }
        }
        else if (nodeNumber == 8)
        {
            static const int quad8LineMap[16] = {
                0, 4, 4, 1, 1, 5, 5, 2, 2, 6, 6, 3, 3, 7, 7, 0,
            };
            res.resize(16);
            for (int i = 0; i < 16; i++)
            {
                auto node = _elemFaceMap[_elemFaceOffset[elemFaceIdx] + quad8LineMap[i]];
                res[i] = _elem.getNodes()[node];
            }
        }
        else
        {
        }
        return res;
    }

} // namespace EgLab::ME
