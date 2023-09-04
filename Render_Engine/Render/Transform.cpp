#include "Transform.h"

Eigen::Matrix4f Transform::GetMatrix()
{
    Eigen::Transform<float,3,Eigen::Affine > t;
    t.matrix() = Eigen::Matrix4f::Identity();
    //rotate to rotation
    //t = Rotation.toRotationMatrix();

    // translate to position
    t.translate(Position);

    // rotate to rotation
    t.rotate(Rotation);
    //Rotation.normalize();
    //t.rotate(Eigen::AngleAxis<float>(Rotation.x(),Eigen::Vector3f::UnitX()));
    //t.rotate(Eigen::AngleAxis<float>(Rotation.y(),Eigen::Vector3f::UnitY()));
    //t.rotate(Eigen::AngleAxis<float>(Rotation.z(),Eigen::Vector3f::UnitZ()));

    /*t.rotate(Eigen::AngleAxis<float>(Rotation.x(),Eigen::Vector3f::UnitX()));
    t.rotate(Eigen::AngleAxis<float>(Rotation.y(),Eigen::Vector3f::UnitY()));
    t.rotate(Eigen::AngleAxis<float>(Rotation.z(),Eigen::Vector3f::UnitZ()));*/

    // scale to Scale
    t.scale(Scale);

    return t.matrix();
    
}

Eigen::Matrix4f Transform::Get_Rotation_Matrix()
{
    Eigen::Transform<float,3,Eigen::Affine > t;
    t.matrix() = Eigen::Matrix4f::Identity();
    //Rotation.normalize();
    //t = Rotation.toRotationMatrix();
    t.rotate(Rotation);
    //t.rotate(Eigen::AngleAxis<float>(Rotation.x(),Eigen::Vector3f::UnitX()));
    //t.rotate(Eigen::AngleAxis<float>(Rotation.y(),Eigen::Vector3f::UnitY()));
    //t.rotate(Eigen::AngleAxis<float>(Rotation.z(),Eigen::Vector3f::UnitZ()));
    return t.matrix();


}

void Transform::SetPosition(Eigen::Vector3f position)
{
    Position = position;

}

