#pragma once

#include <eigen3/Eigen/Eigen>
#include <eigen3/Eigen/Geometry>

class Transform
{
public:
    Eigen::Vector3f Position;
    //Eigen::Vector3f Rotation;
    Eigen::Quaternion<float> Rotation;
    Eigen::Vector3f Scale;
public:
    Transform()
    {
        Position << 0, 0, 0;
        Rotation = Eigen::Quaternion<float>(1, 0, 0, 0);
        Scale    << 1, 1, 1;
    }
    Transform(Eigen::Vector3f position,
              Eigen::Vector3f rotation,
              Eigen::Vector3f scale
        ) : Position(position),Scale(scale)
    {
        Eigen::AngleAxisf rollAngle(Eigen::AngleAxisf(rotation(2),Eigen::Vector3f::UnitX()));
        Eigen::AngleAxisf pitchAngle(Eigen::AngleAxisf(rotation(1),Eigen::Vector3f::UnitY()));
        Eigen::AngleAxisf yawAngle(Eigen::AngleAxisf(rotation(0),Eigen::Vector3f::UnitZ()));
         
        Rotation=yawAngle*pitchAngle*rollAngle;
    }

    /*inline Eigen::Vector3f GetPosition() const{ return Position; }
    inline Eigen::Vector3f GetRotation() const{ return Rotation; }
    inline Eigen::Vector3f GetScale() const{ return Scale; }*/

    Eigen::Matrix4f GetMatrix();
    Eigen::Matrix4f Get_Rotation_Matrix();

    void SetPosition(Eigen::Vector3f position);
};
