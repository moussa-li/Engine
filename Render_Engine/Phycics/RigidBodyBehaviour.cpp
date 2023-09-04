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
    //m_ComputeCollisionShader->Bind();

    

    //GLvoid *p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);//获取着色器中buffer块的内存地址
	//memcpy(p, &collision_number, sizeof(collision_number));//将结构体数据拷贝到着色器buffer块地址上
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);//释放内存地址

    //m_ComputeCollisionShader->Unbind();



    for (size_t i = 0; i < m_Meshes.size(); ++i)
    {
        //std::vector<Eigen::Vector3f>  &vertices = ((Mesh*)m_Meshes[i])->m_Vertices;
        float* vertices = ((Mesh*)m_Meshes[i])->Vertices;
        
        m_Mass = 0;
        float m = 1;
        for (size_t j = 0; j < ((Mesh*)m_Meshes[i])->Vertices_Length; j+=3)
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
    R = ((Transform*)m_Transform)->Get_Rotation_Matrix();

    total_Length = 0;
    for (size_t i = 0; i < m_Meshes.size(); ++i)
    {
        total_Length += ((Mesh*)m_Meshes[i])->Vertices_Length;
        //std::vector<Eigen::Vector3f> &tmp_Vertives = ((Mesh*)m_Meshes[i])->m_Vertices;
        //vertices.insert(vertices.end(), tmp_Vertives.begin(), tmp_Vertives.end());
    }
    size_t offset = 0;
    std::vector<float> vertices(total_Length);
    //vertices = new float[total_Length];
    for (size_t i = 0; i < m_Meshes.size(); ++i)
    {
        //memcpy(&vertices+offset, ((Mesh*)m_Meshes[i])->Vertices, ((Mesh*)m_Meshes[i])->Vertices_Length);
        vertices.assign(((Mesh*)m_Meshes[i])->Vertices, (((Mesh*)m_Meshes[i])->Vertices + ((Mesh*)m_Meshes[i])->Vertices_Length));
        offset += ((Mesh*)m_Meshes[i])->Vertices_Length;
    }

    m_ComputeCollisionShader->Bind();
    GLCall(glGenBuffers(1, &ssbo4));
    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo4));
    GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * total_Length, vertices.data(), GL_DYNAMIC_COPY));
    //input = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    //memcpy(input, vertices, sizeof(float)*total_Length);
    GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo4));
    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
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
        V = Eigen::Vector3f(5, 1, 0);
        launched = true;
        //V = Eigen::
    }
    if (launched)
    {
        V += Eigen::Vector3f(0, m_Gravity * dt, 0);
        Collision_Impulse(Eigen::Vector3f(0, -0.9f, 0),Eigen::Vector3f(0, 1.f, 0));
        Collision_Impulse(Eigen::Vector3f(1.9f, 0, 0),Eigen::Vector3f(-1.f, 0, 0));
        V *= m_LinearDecay;
        
        Eigen::Vector3f x = transform->Position + V * dt;

        W *= m_AngularDecay;
        Eigen::Vector3f dw = 0.5f * dt * W;
        Eigen::Quaternion q = Add(transform->Rotation , Eigen::Quaternion<float>(0.f,dw.x(), dw.y(), dw.z()) * transform->Rotation);

        //Log::Log(Log::Debug, x);
        //Log::Log(Log::Debug, q);

        transform->Position = x;
        transform->Rotation = q;
        //Print(x);

        R = transform->Get_Rotation_Matrix();
    }
    
}

void PhyE::RigidBodyBehaviour::Collision_Impulse(Eigen::Vector3f P, Eigen::Vector3f N)
{
    Transform* transform = (Transform*)m_Transform;
    //std::vector<Eigen::Vector3f> vertices = {};
    
    
    //float phi_x = 0;
    
    Eigen::Vector3f sum = Eigen::Vector3f(0, 0, 0);
    int sum_intege[3] = { 0,0,0 };
        float sum_x=0.f, sum_y = 0.f, sum_z = 0.f;
    //int collision_number = 0;
    float  firstTime = static_cast<float>(glfwGetTime());
    int collision_number = 0;

    m_ComputeCollisionShader->Bind();
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(collision_number), &collision_number, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


    glGenBuffers(1, &ssbo2);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo2);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 3, sum_intege, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo2);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLCall(glGenBuffers(1, &ssbo3));
    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo3));
    struct InputParam {
        Eigen::Vector4f position;
        Eigen::Vector4f P;
        Eigen::Vector4f V;
        Eigen::Vector4f W;
        Eigen::Vector4f N;
    } inputparam;
    inputparam.position = transform->Position.homogeneous();
    inputparam.P = P.homogeneous();
    inputparam.V = V.homogeneous();
    inputparam.W = W.homogeneous();
    inputparam.N = N.homogeneous();
    GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(inputparam), nullptr, GL_DYNAMIC_COPY));
    float * input = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memcpy(input, &inputparam, sizeof(inputparam));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo3));
    GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

    //GLCall(glGenBuffers(1, &ssbo4));
    //GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo4));
    //GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * total_Length, vertices.data(), GL_DYNAMIC_COPY));
    ////input = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    ////memcpy(input, vertices, sizeof(float)*total_Length);
    //GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo4));
    //GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

    glGenBuffers(1, &ssbo5);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo5);
    GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(R), R.data(), GL_DYNAMIC_COPY));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo5);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &ssbo6);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo6);
    GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, (sizeof(float)+sizeof(int)+sizeof(int))*total_Length/3, nullptr, GL_DYNAMIC_COPY));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssbo6);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    //GLCall(glDispatchCompute(2, 1, 1));
    int group_x = (total_Length / 3 + 1024) / 1024;
    GLCall(glDispatchCompute(group_x, 1, 1));
    GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BUFFER));

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    int* output = (int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    collision_number = *output;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo2);
    int* output_sum = (int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    sum = Eigen::Vector3f(((float)*output_sum)/100000, ((float)*(output_sum+1))/100000,((float)*(output_sum+2))/100000);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo4);
    float* output_param = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo5);
    //debug = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    //memcpy(&R, debug, sizeof(inputparam));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo6);
    unsigned int *debug;
    debug = (unsigned int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    //memcpy(&R, debug, sizeof(inputparam));
    if (collision_number != 0)
    {
        printf("");

    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    /*glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo3);
    output_sum = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    Eigen::Vector3f p(output_sum[0], output_sum[1], output_sum[2]);
    Eigen::Matrix4f r(&output_sum[3]);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);*/

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    //#pragma omp parallel for num_threads(128) reduction(+:collision_number) reduction(+:sum_x) reduction(+:sum_y) reduction(+:sum_z)
    //for (int i = 0; i < total_Length/3; i++)
    //{
    //    Eigen::Vector3f vertex(vertices[i], vertices[i + 1], vertices[i + 2]);
    //    //Eigen::Vector3f Xi = transform->Position + Mat4f_Multiply_Vec3(R, vertex) ;
    //    Eigen::Vector3f Xi = transform->Position + (R.block<3,3>(0,0) * vertex);
    //    
    //    float phi_x = Signed_Distance_Plane(P, N, Xi);
    //    //float phi_x = Xi.y() - -1.f;
    //    //float phi_x = -2;
    //    if (phi_x < 0)
    //    {
    //        Eigen::Vector3f Vi = V + W.cross(R.block<3,3>(0,0) * vertex);
    //        if (Vi.dot(N) < 0)
    //        {
    //            //sum += vertex;
    //            sum_x += vertex.x();
    //            sum_y += vertex.y();
    //            sum_z += vertex.z();
    //            collision_number++;
    //        }
    //    }
    //}

    //delete[] vertices;
    m_ComputeCollisionShader->Unbind();

    //sum = Eigen::Vector3f(sum_x, sum_y, sum_z);
    float  secondTime = static_cast<float>(glfwGetTime());
    if (collision_number == 0)return;
    Log::Log(Log::Debug, std::to_string(collision_number));    

    Eigen::Matrix4f I_rot = R * I_ref * R.transpose();
    Eigen::Matrix4f I_inverse = I_rot.inverse();
    Eigen::Vector3f r_collision = sum / collision_number;
    Print(sum);
    Eigen::Vector3f Rr_collision = (R * r_collision.homogeneous()).head<3>();
    Eigen::Vector3f V_collision = V + W.cross(Rr_collision);

    Eigen::Vector3f V_n = (V_collision.dot(N)) * N;
    Eigen::Vector3f V_t = V_collision - V_n;

    Eigen::Vector3f V_n_new = -m_Restitution * V_n;
    float a = std::max(1.f - (m_Friction * (1.f + m_Restitution) * V_n.norm() / V_t.norm()) , 0.f);
    //Log::Log(Log::Debug, std::to_string(a));
    //Log::Log(Log::Debug, std::to_string(V_t.norm()));
    //Log::Log(Log::Debug, std::to_string(1.f - (m_Friction * (1.f + m_Restitution) * V_n.norm() / V_t.norm())));
    Eigen::Vector3f V_t_new = a * V_t;
    Eigen::Vector3f V_new = V_n_new + V_t_new;

    Eigen::Matrix4f Rri_star = Get_Cross_Matrix(Rr_collision);
    Eigen::Matrix4f K = (1.f / m_Mass * Eigen::Matrix4f::Identity()) - (Rri_star * I_inverse * Rri_star);
    Eigen::Vector4f V_collision_new;
    V_collision_new << (V_new - V_collision), 1.0f;
    Eigen::Vector3f J = (K.inverse() * (V_collision_new)).head<3>();

    V += 1.f / m_Mass * J;
    Eigen::Vector4f Rr_collisionxJ;
    Rr_collisionxJ << Rr_collision.cross(J), 1.0f;
    W += (I_inverse * Rr_collisionxJ).head<3>();
    //Print(J);
    //Print(collision_number);

    m_Restitution *= 0.5f;
}

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

Eigen::Quaternion<float> PhyE::RigidBodyBehaviour::Add(Eigen::Quaternion<float> a, Eigen::Quaternion<float> b)
{
    Eigen::Quaternion<float> result;
    result.x() = a.x() + b.x();
    result.y() = a.y() + b.y();
    result.z() = a.z() + b.z();
    result.w() = a.w() + b.w();
    return result;

}

Eigen::Vector3f PhyE::RigidBodyBehaviour::Mat4f_Multiply_Vec3(Eigen::Matrix4f mat, Eigen::Vector3f vec)
{
    Eigen::Vector3f result;
    result.x() = mat(0, 0) * vec.x() + mat(0, 1) * vec.y() + mat(0, 2) * vec.z();
    result.x() = mat(1, 0) * vec.x() + mat(1, 1) * vec.y() + mat(1, 2) * vec.z();
    result.x() = mat(2, 0) * vec.x() + mat(2, 1) * vec.y() + mat(2, 2) * vec.z();
    return result;

}

