#include "Transform.hpp"

namespace EgLab
{
    Transform::Transform()
    {
    }

    Transform::Transform(CoordType position, CoordType rotation, CoordType scale)
    {
    }

    Matrix4f Transform::getMatrix()
    {
        return Matrix4f();
    }

    Matrix4f Transform::getRotationMatrix()
    {
        return Matrix4f();
    }

    void Transform::setPosition(CoordType positoin)
    {
    }
} // namespace EgLab