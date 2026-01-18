#pragma once

#include "Common/Vector.hpp"
#include "Definites.hpp"

namespace EgLab
{
    class Transform
    {
    public:
        Transform();

        Transform(CoordType position, CoordType rotation, CoordType scale);

        Matrix4f getMatrix();
        Matrix4f getRotationMatrix();

        void setPosition(CoordType positoin);

    private:
        CoordType _position;
        CoordType _rotation;
        CoordType _scale;
    };

} // namespace EgLab
