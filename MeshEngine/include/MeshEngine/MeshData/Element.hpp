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
} // namespace EgLab::ME