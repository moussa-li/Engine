#include <math.h>
#include <string>

#include "RigidBodyBehaviour.h"
#include "Transform.h"
#include "Mesh.h"
#include "Collision.h"

#include "Print.h"


PhyE::RigidBodyBehaviour::RigidBodyBehaviour(void* transform, float restitution, float friction, float gravity /*= -9.8f*/)
    : Behaviour(transform, restitution, friction)
{
}

void PhyE::RigidBodyBehaviour::Start()
{
    m_ComputeCollisionShader = new Shader("res/shaders/Compute_Collision.shader");

    /* compute the refer of  rotational inertia */
    for (size_t i = 0; i < m_Meshes.size(); ++i)
    {
        Mesh* mesh = (Mesh*)m_Meshes[i];
        float* vertices = (mesh)->Vertices;
        
        m_Mass = 0;
        float m = 1;
        for (size_t j = 0; j < (mesh)->Vertices_Length; j+=3)
        {
            m_Mass += m;
            float diag = m * (vertices[j]*vertices[j] + vertices[j+1]*vertices[j+1] +vertices[j+2]*vertices[j+2]);
            I_ref(0, 0) += diag;
            I_ref(1, 1) += diag;
            I_ref(2, 2) += diag;
            I_ref(0, 0) -= m * vertices[j] * vertices[j];
            I_ref(0, 1) -= m * vertices[j] * vertices[j+1];
            I_ref(0, 2) -= m * vertices[j] * vertices[j+2];
            I_ref(1, 0) -= m * vertices[j+1] * vertices[j];
            I_ref(1, 1) -= m * vertices[j+1] * vertices[j+1];
            I_ref(1, 2) -= m * vertices[j+1] * vertices[j+2];
            I_ref(2, 0) -= m * vertices[j+2] * vertices[j];
            I_ref(2, 1) -= m * vertices[j+2] * vertices[j+1];
            I_ref(2, 2) -= m * vertices[j+2] * vertices[j+2];
        }
    }
    I_ref(3, 3) = 1;

    // Entity's rotation
    R = ((Transform*)m_Transform)->Get_Rotation_Matrix();

    /* Input the vertices of mesh to the GPU memory */
    total_Length = 0;
    for (size_t i = 0; i < m_Meshes.size(); ++i) // traverse every meshes to compute all meshes' vertices of the entity
    {
        total_Length += ((Mesh*)m_Meshes[i])->Vertices_Length;
    }
    size_t offset = 0;
    float* vertices = new float[total_Length];
    for (size_t i = 0; i < m_Meshes.size(); ++i) // combine every meshes of the vertices
    {
        Mesh* mesh = (Mesh*)m_Meshes[i];
        memcpy(vertices + offset, (mesh)->Vertices, sizeof(float) * mesh->Vertices_Length);
        offset += mesh->Vertices_Length;
    }

    m_ComputeCollisionShader->Bind();
    VerticesBuffer = new ShaderStorageBuffer(sizeof(float) * total_Length, 4);
    VerticesBuffer->InputData(vertices, sizeof(float) * total_Length);
    delete[] vertices;

    OutputCollisionNumber = new ShaderStorageBuffer(sizeof(int), 1);
    OutputSumVec = new ShaderStorageBuffer(sizeof(int) * 3, 2);
    InputParamBuffer = new ShaderStorageBuffer(sizeof(InputParam),3);
    InputR = new ShaderStorageBuffer(sizeof(R), 5);
    
    m_ComputeCollisionShader->Unbind();
}

void PhyE::RigidBodyBehaviour::Update(float deltaTime)
{
    Transform* transform = (Transform*)m_Transform;
    if (glfwGetKey(m_Window, GLFW_KEY_R) == GLFW_PRESS)
    {
        transform->SetPosition(Eigen::Vector3f(0, 0, 0));
        m_Restitution = 0.5f;
        V = Eigen::Vector3f(0, 0, 0);
        W = Eigen::Vector3f(0, 0, 0);
        launched = false;
    }
    if (glfwGetKey(m_Window, GLFW_KEY_L) == GLFW_PRESS)
    {
        V = Eigen::Vector3f(0, 0, 0);
        launched = true;
    }
    if (launched)
    {
        // STEP1: if we don't have any collision how speed change
        V += Eigen::Vector3f(0, m_Gravity * deltaTime, 0);

        // STEP2: certainly we don't always miss anything ,So collision detection and get new velocity
        Collision_Impulse(Eigen::Vector3f(0, -0.99f, 0),Eigen::Vector3f(0, 1.f, 0));
        Collision_Impulse(Eigen::Vector3f(1.99f, 0, 0),Eigen::Vector3f(-1.f, 0, 0));

        // STEP3: we got new linear velocity and angular velocity , so what position we will get next time
        V *= m_LinearDecay;
        Eigen::Vector3f x = transform->Position + V * deltaTime;

        W *= m_AngularDecay;
        Eigen::Vector3f dw = 0.5f * deltaTime * W;
        Eigen::Quaternion q = Add(transform->Rotation , Eigen::Quaternion<float>(0.f,dw.x(), dw.y(), dw.z()) * transform->Rotation);

        transform->Position = x;
        transform->Rotation = q;

        R = transform->Get_Rotation_Matrix();
    }
    
}

void PhyE::RigidBodyBehaviour::Collision_Impulse(Eigen::Vector3f P, Eigen::Vector3f N)
{
    Transform* transform = (Transform*)m_Transform;

    Eigen::Vector3f SumVec = Eigen::Vector3f(0, 0, 0);
    int SumVeci[3] = { 0,0,0 };

    int CollisionNumber = 0;

    m_ComputeCollisionShader->Bind();
    
    /* input the data what compute shader need */

    OutputCollisionNumber->InputData(&CollisionNumber, sizeof(int));

    OutputSumVec->InputData(SumVeci, sizeof(int) * 3);
    
    InputParam.position = transform->Position.homogeneous();
    InputParam.P = P.homogeneous();
    InputParam.V = V.homogeneous();
    InputParam.W = W.homogeneous();
    InputParam.N = N.homogeneous();

    InputParamBuffer->InputData(&InputParam, sizeof(InputParam));
 
    InputR->InputData(R.data(), sizeof(R));
  
    /* Parallel compute */
    int group_x = (total_Length / 3 + 1024) / 1024;
    GLCall(glDispatchCompute(group_x, 1, 1));
    GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BUFFER));

    /* The data is back , how many vertices have already gone through */
    OutputCollisionNumber->OutputData(&CollisionNumber);
    
    OutputSumVec->OutputData(SumVeci);
    SumVec << (float)SumVeci[0] / 100000, (float)SumVeci[1] / 100000, (float)SumVeci[2] / 100000;
    
    m_ComputeCollisionShader->Unbind();

    if (CollisionNumber == 0)return;
    // if we detection the collision we need do something , here i use the Impulse Method

    /* Get current rotation of the inertia */
    Eigen::Matrix4f I_rot = R * I_ref * R.transpose();
    Eigen::Matrix4f I_inverse = I_rot.inverse();
    /* We need the average of the collision's position */
    Eigen::Vector3f r_collision = SumVec / CollisionNumber;
    /* Rotation! don't forget that please */
    Eigen::Vector3f Rr_collision = (R * r_collision.homogeneous()).head<3>();
    /* Thus we can get the native velocity of the collision */
    Eigen::Vector3f V_collision = V + W.cross(Rr_collision);

    /* we need to divided into normal velocity and tangent velocity , Simplify complex problems :) */
    Eigen::Vector3f V_n = (V_collision.dot(N)) * N;
    Eigen::Vector3f V_t = V_collision - V_n;

    /* there is always some slowdown */
    Eigen::Vector3f V_n_new = -m_Restitution * V_n;
    float a = std::max(1.f - (m_Friction * (1.f + m_Restitution) * V_n.norm() / V_t.norm()) , 0.f);
    Eigen::Vector3f V_t_new = a * V_t;
    Eigen::Vector3f V_new = V_n_new + V_t_new;

    /* Get the Impulse Orz */
    Eigen::Matrix4f Rri_star = Get_Cross_Matrix(Rr_collision);
    Eigen::Matrix4f K = (1.f / m_Mass * Eigen::Matrix4f::Identity()) - (Rri_star * I_inverse * Rri_star);
    Eigen::Vector4f V_collision_new;
    V_collision_new << (V_new - V_collision), 1.0f;
    Eigen::Vector3f J = (K.inverse() * (V_collision_new)).head<3>();

    /* I = mv then v = m^-1*I */
    V += 1.f / m_Mass * J;
    /* Round round and round */
    Eigen::Vector4f Rr_collisionxJ;
    Rr_collisionxJ << Rr_collision.cross(J), 1.0f;
    W += (I_inverse * Rr_collisionxJ).head<3>();
    
    /* we need to reduce the restitution in order to reduce the jitter */
    m_Restitution *= 0.9f;
}

// there are interesting that vec(a) cross vec(b) could convert to matrix(a) dot vec(b)
Eigen::Matrix4f PhyE::RigidBodyBehaviour::Get_Cross_Matrix(Eigen::Vector3f a)
{
    Eigen::Matrix4f result = Eigen::Matrix4f::Zero();
    result(0, 0) = 0;
    result(0, 1) = -a(2);
    result(0, 2) = a(1);
    result(1, 0) = a(2);
    result(1, 1) = 0;
    result(1, 2) = -a(0);
    result(2, 0) = -a(1);
    result(2, 1) = a(0);
    result(2, 2) = 0;
    result(3, 3) = 1;
    return result;

}

// why the Eigen of Quaternion don't have add operator
Eigen::Quaternion<float> PhyE::RigidBodyBehaviour::Add(Eigen::Quaternion<float> a, Eigen::Quaternion<float> b)
{
    Eigen::Quaternion<float> result;
    result.x() = a.x() + b.x();
    result.y() = a.y() + b.y();
    result.z() = a.z() + b.z();
    result.w() = a.w() + b.w();
    return result;

}

// Maybe we dont's need this function 
Eigen::Vector3f PhyE::RigidBodyBehaviour::Mat4f_Multiply_Vec3(Eigen::Matrix4f mat, Eigen::Vector3f vec)
{
    Eigen::Vector3f result;
    result.x() = mat(0, 0) * vec.x() + mat(0, 1) * vec.y() + mat(0, 2) * vec.z();
    result.x() = mat(1, 0) * vec.x() + mat(1, 1) * vec.y() + mat(1, 2) * vec.z();
    result.x() = mat(2, 0) * vec.x() + mat(2, 1) * vec.y() + mat(2, 2) * vec.z();
    return result;

}

