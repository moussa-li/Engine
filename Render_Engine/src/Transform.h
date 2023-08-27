#pragma once

#include <eigen3/Eigen/Eigen>
#include <eigen3/Eigen/Geometry>

class Transform
{
private:
    Eigen::Vector3f Position;
    Eigen::Vector3f Rotation;
    Eigen::Vector3f Scale;
public:
    Transform()
    {
        Position << 0, 0, 0;
        Rotation << 0, 0, 0;
        Scale    << 1, 1, 1;
    }
    Transform(Eigen::Vector3f position,
              Eigen::Vector3f rotation,
              Eigen::Vector3f scale
        ) : Position(position),Rotation(rotation),Scale(scale)
    {}

    inline Eigen::Vector3f GetPosition() const{ return Position; }
    inline Eigen::Vector3f GetRotation() const{ return Rotation; }
    inline Eigen::Vector3f GetScale() const{ return Scale; }

    Eigen::Matrix4f GetMatrix();
};
