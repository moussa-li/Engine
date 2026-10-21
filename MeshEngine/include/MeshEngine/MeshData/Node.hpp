#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "MeshEngine/Common/Definitions.hpp"
#include "MeshEngine/Common/SoaContainer.hpp"

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
        friend struct SoaRegistrar<Node>;
        IdType _id;
        CoordType _coord;
    };

    BEGIN_SOA_REG(Node)
    SOA_REGISTERED_TYPE(Node)
    REG_COL(IdType, _id)
    REG_COL(CoordType, _coord)
    END_SOA_REG()

    using Nodes = ME::SoaContainer<Node>;

} // namespace EgLab::ME