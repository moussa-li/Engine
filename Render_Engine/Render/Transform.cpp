#include "Transform.h"

Eigen::Matrix4f Transform::GetMatrix()
{
    Eigen::Transform<float,3,Eigen::Affine > t;
    t.matrix() = Eigen::Matrix4f::Identity();

    // translate to position
    t.translate(Position);

    // rotate to rotation
    Rotation.normalize();
    t.rotate(Rotation);
    
    // scale to Scale
    t.scale(Scale);

    return t.matrix();
    
}

Eigen::Matrix4f Transform::Get_Rotation_Matrix()
{
    Eigen::Transform<float,3,Eigen::Affine > t;
    t.matrix() = Eigen::Matrix4f::Identity();
    Rotation.normalize();
    t.rotate(Rotation);
    
    return t.matrix();


}

void Transform::SetPosition(Eigen::Vector3f position)
{
    Position = position;

}

