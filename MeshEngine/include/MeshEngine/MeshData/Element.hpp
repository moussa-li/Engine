#pragma once

#include "Common/DynamicArray.hpp"
#include "MeshEngine/Common/Definitions.hpp"

namespace EgLab::ME
{
    enum class ElemType : int
    {
        Tri3 = 0x203,
        Tri6 = 0x206,
        Quad4 = 0x204,
        Quad8 = 0x208,
        Tet4 = 0x304,
        Tet10 = 0x30a,
        Wed6 = 0x306,
        Prd5 = 0x305,
        Hex8 = 0x308,
        Hex20 = 0x314
    };

    int getTypeDim(ElemType type);

    class Elem
    {
    public:
        IdType getId() const
        {
            return _id;
        }

        ElemType getType() const
        {
            return _type;
        }

        void setId(IdType id)
        {
            _id = id;
        }

        void setType(ElemType type)
        {
            _nodes.resize(getTypeDim(type));
            _type = type;
        }

        void setNodes(Common::DynamicArray<IdType> &&nodes)
        {
            _nodes = Common::move(nodes);
        }

        const Common::DynamicArray<IdType> &getNodes() const
        {
            return _nodes;
        }

        IdType getNode(IdxType idx)
        {
            return _nodes[idx];
        }

        void setNode(IdxType idx, IdType id)
        {
            _nodes[idx] = id;
        }

    private:
        IdType _id;
        ElemType _type;
        Common::DynamicArray<IdType> _nodes;
    };

    class ElemFaceIterator
    {
    public:
        ElemFaceIterator(const Elem &);

    private:
        const Elem &_elem;
    };

    class ElemFaceGetter
    {
    public:
        ElemFaceGetter(const Elem &);
        int elemFaceNumber() const;

        int elemFaceNodeNumber(IdxType elemFaceIdx) const;

        IdType elemFace(IdxType elemFaceIdx, IdxType nodeIdx) const;

        Common::DynamicArray<IdType> getNodes(IdxType) const;

        Common::DynamicArray<IdType> getLineNodes(IdxType) const;

    private:
        const Elem &_elem;
        int *_elemFaceNodeNumberMap;
        int *_elemFaceOffset;
        IdxType *_elemFaceMap;
    };
} // namespace EgLab::ME