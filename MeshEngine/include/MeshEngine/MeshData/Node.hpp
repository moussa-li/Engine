#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "MeshEngine/Common/Definitions.hpp"

namespace EgLab::ME
{
    class Node
    {
    public:
        void setId(IdType id)
        {
            _id = id;
        }

        IdType getId()
        {
            return _id;
        };

        void setXYZ(CoordType& xyz)
        {
            _coord = xyz;
        }

        Scalar x() const
        {
            return _coord.x();
        }

        Scalar y() const
        {
            return _coord.y();
        }

        Scalar z() const
        {
            return _coord.z();
        }

        void setX(Scalar x)
        {
            _coord.x() = x;
        }

        void setY(Scalar y)
        {
            _coord.y() = y;
        }

        void setZ(Scalar z)
        {
            _coord.z() = z;
        }

    private:
        IdType _id;
        CoordType _coord;
    };

    // class Nodes
    // {
    // public:
    //     Common::Return addNodes(IdType, Node);

    //     Common::Return delNodes(IdType);

    //     Common::Return setNodes(IdType, Node);

    // private:
    //     Common::DynamicArray<Node> _nodes;
    //     Common::DynamicArray<IdType> _ids;
    // };

} // namespace EgLab::ME