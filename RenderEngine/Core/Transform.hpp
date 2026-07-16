#pragma once

#include "Common/Vector.hpp"
#include "Definites.hpp"

namespace EgLab
{
    class Transform
    {
    public:
        Transform();

        ~Transform() = default;

        Transform(CoordType position, CoordType rotation, CoordType scale);

        Transform(const Transform &other);

        Matrix4f getMatrix();
        Matrix4f getRotationMatrix();

        void setPosition(CoordType positoin);

    private:
        CoordType _position;
        CoordType _rotation;
        CoordType _scale;
    };

} // namespace EgLab
