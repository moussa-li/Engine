#pragma once

#include <eigen3/Eigen/Eigen>
#include <vector>
#include <unordered_set>
#include <filesystem>

#include "VertexArray.h"  //VAO
#include "VertexBuffer.h" //VBO
#include "IndexBuffer.h"  //EBO
#include "Vertex.h"
#include "VertexBufferLayout.h"
#include "Texture.h"
#include "Shader.h"
#include "ShaderStorageBuffer.h"

#include "Transform.h"

#include "libexport.h"

enum MeshType {
    Tet
};

struct Vector3fHash {
    std::size_t operator()(const Eigen::Vector3f& vec) const {
        std::size_t h1 = std::hash<float>{}(vec.x());
        std::size_t h2 = std::hash<float>{}(vec.y());
        std::size_t h3 = std::hash<float>{}(vec.z());
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

// 相等性比较函数，用于比较两个Eigen::Vector3f对象是否相等
struct Vector3fEqual {
    bool operator()(const Eigen::Vector3f& lhs, const Eigen::Vector3f& rhs) const {
        return lhs.isApprox(rhs); // 使用Eigen的isApprox函数进行近似比较
    }
};

class DLLAPI Mesh
{
public:
    std::vector<Eigen::Vector3f>  m_Vertices; // vertices coordinates
    std::vector<Eigen::Vector3f>  m_Normals; // vertices normal
    std::vector<Eigen::Vector2f>  m_TexCoords; // vertices textures coordinates
    std::vector<size_t>  m_indices; // element index array
    std::vector<Texture*> m_Texture; // element texture array

    float* Vertices;
    size_t Vertices_Length;
    
public:
    VertexArray  *VAO;
    VertexArray  *Normal_VAO;
    VertexArray  *Texture_VAO;
    VertexBuffer *VBO;
    VertexBuffer *Normal_VBO;
    VertexBuffer *Texture_VBO;
    IndexBuffer  *EBO;

    static Shader* RecalculateNormalsShader;
    static ShaderStorageBuffer* VerticesBuffer;
    static ShaderStorageBuffer* IndicesBuffer;
    static ShaderStorageBuffer* NormalsBuffer;


    MeshType m_MeshType;

public:
    Mesh() {
        m_MeshType = MeshType::Tet;
    }

    Mesh(std::vector<Eigen::Vector3f> vertices, std::vector<Eigen::Vector3f> normals, std::vector<Eigen::Vector2f> texcoords, std::vector<size_t> indices, std::vector<Texture*> texture)
    {
        m_MeshType = Tet;
        m_Vertices = vertices;
        m_Normals = normals;
        m_TexCoords = texcoords;
        m_indices = indices;
        m_Texture = texture;


        Setup();
    }

    void Insert_Texture(Texture* texture);

    virtual void Setup();
    void Clear();
    virtual void Draw(Shader *shader, Transform transform);
    void Update();
    void RecalculateNormals();

    ~Mesh();

};
