#include "Transform.h"

Eigen::Matrix4f Transform::GetMatrix()
{
    Eigen::Transform<float,3,Eigen::Affine > t;
    t.matrix() = Eigen::Matrix4f::Identity();
    // translate to position
    t.translate(Position);

    // rotate to rotation
    t.rotate(Eigen::AngleAxis<float>(Rotation.x(),Eigen::Vector3f::UnitX()));
    t.rotate(Eigen::AngleAxis<float>(Rotation.y(),Eigen::Vector3f::UnitY()));
    t.rotate(Eigen::AngleAxis<float>(Rotation.z(),Eigen::Vector3f::UnitZ()));

    // scale to Scale
    t.scale(Scale);

    return t.matrix();
    
}

Eigen::Matrix4f Transform::Get_Rotation_Matrix()
{
    Eigen::Transform<float,3,Eigen::Affine > t;
    t.matrix() = Eigen::Matrix4f::Identity();
    t.rotate(Eigen::AngleAxis<float>(Rotation.x(),Eigen::Vector3f::UnitX()));
    t.rotate(Eigen::AngleAxis<float>(Rotation.y(),Eigen::Vector3f::UnitY()));
    t.rotate(Eigen::AngleAxis<float>(Rotation.z(),Eigen::Vector3f::UnitZ()));
    return t.matrix();


}

void Transform::SetPosition(Eigen::Vector3f position)
{
    Position = position;

}

