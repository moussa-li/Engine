#pragma once

#include <eigen3/Eigen/Eigen>
#include <vector>
#include <filesystem>

#include "VertexArray.h"  //VAO
#include "VertexBuffer.h" //VBO
#include "IndexBuffer.h"  //EBO
#include "Vertex.h"
#include "VertexBufferLayout.h"
#include "Texture.h"
#include "Shader.h"

#include "Transform.h"

#include "libexport.h"

enum MeshType {
    Tet
};

class DLLAPI Mesh
{
private:
    std::vector<Vertex>  m_Vertices;
    std::vector<size_t>  m_indices;
    std::vector<Texture*> m_Texture;
    
    VertexArray  *VAO;
    VertexArray  *Normal_VAO;
    VertexArray  *Texture_VAO;
    VertexBuffer *VBO;
    VertexBuffer *Normal_VBO;
    VertexBuffer *Texture_VBO;
    IndexBuffer  *EBO;


    MeshType m_MeshType;

public:
    Mesh() {
        m_MeshType = MeshType::Tet;
    }

    Mesh(std::vector<Vertex> vertex, std::vector<size_t> indices, std::vector<Texture*> texture)
    {
        m_MeshType = Tet;
        m_Vertices = vertex;
        m_indices = indices;
        m_Texture = texture;

        Setup();
    }

    void Insert_Texture(Texture* texture);

    virtual void Setup();
    virtual void Draw(Shader *shader, Transform transform);

    ~Mesh() {
        if (VAO)
        {
            delete VAO;
        }
        if (VBO)
        {
            delete VBO;
        }
        if (EBO)
        {
            delete EBO;
        }
    }

};
