#include "Transform.hpp"

namespace EgLab::RE
{
    Transform::Transform()
    {
    }

    Transform::Transform(CoordType position, CoordType rotation, CoordType scale)
        : _position(position), _rotation(rotation), _scale(scale)
    {
    }

    Transform::Transform(const Transform &other)
    {
        _position = other._position;
        _rotation = other._rotation;
        _scale = other._scale;
    }

    Common::Matrix4f Transform::getMatrix()
    {
        Common::Matrix4f pos = Common::Matrix4f::Identity();
        pos[0][3] = _position[0];
        pos[1][3] = _position[1];
        pos[2][3] = _position[2];

        Common::Matrix4f scale;
        scale[0][0] = _scale[0];
        scale[1][1] = _scale[1];
        scale[2][2] = _scale[2];

        return pos * scale * getRotationMatrix();
    }

    Common::Matrix4f Transform::getRotationMatrix()
    {
        Common::Matrix4f m;
        float pitch = _rotation.x();
        float yaw = _rotation.y();
        float roll = _rotation.z();

        float cp = cosf(pitch), sp = sinf(pitch);
        float cy = cosf(yaw), sy = sinf(yaw);
        float cr = cosf(roll), sr = sinf(roll);

        // ZYX
        // Row 0
        m[0][0] = cy * cr;
        m[0][1] = cy * sr;
        m[0][2] = -sy;
        m[0][3] = 0.0f;

        // Row 1
        m[1][0] = sp * sy * cr - cp * sr;
        m[1][1] = sp * sy * sr + cp * cr;
        m[1][2] = sp * cy;
        m[1][3] = 0.0f;

        // Row 2
        m[2][0] = cp * sy * cr + sp * sr;
        m[2][1] = cp * sy * sr - sp * cr;
        m[2][2] = cp * cy;
        m[2][3] = 0.0f;

        // Row 3
        m[3][0] = 0.0f;
        m[3][1] = 0.0f;
        m[3][2] = 0.0f;
        m[3][3] = 1.0f;

        return m;
    }

    void Transform::setPosition(CoordType positoin)
    {
    }
} // namespace EgLab::RE