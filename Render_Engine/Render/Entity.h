#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <eigen3/Eigen/Eigen>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Transform.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"

#include "Behaviour.h"

#include "libexport.h"

class DLLAPI Entity
{
protected:
    Transform m_Transform;
    std::vector<Mesh*> m_Meshes;
    std::string m_fileDir;
    std::vector<Texture*> textures_loaded;
    Shader *m_Shader;

    PhyE::Behaviour *m_Behaviour;

    Eigen::Vector3f v;

    GLFWwindow* m_Window;

public:
    Entity() {}
    Entity(const std::string& filepath);
    
    Entity(Eigen::Vector3f position, Eigen::Vector3f rotation, Eigen::Vector3f scale);
    Entity(Eigen::Vector3f position, Eigen::Vector3f rotation, Eigen::Vector3f scale, std::string ShaderPath);


    virtual void Load(const std::string & fileDir);

    virtual void Draw();

    virtual void Start();

    virtual void Update(float deltaTime);

    void Set_Window(GLFWwindow* window);

    void Shader_Load_Camera(Eigen::Matrix4f &proj, Eigen::Matrix4f &view);

    inline Shader *Get_Shader() { return m_Shader; } 

    virtual inline std::string Get_Type() { return "Entity"; }


    virtual ~Entity()
    {
        if (m_Shader)
            delete m_Shader;
        for (size_t i = 0; i < m_Meshes.size(); ++i)
            delete m_Meshes[i];
        for (size_t i = 0; i < textures_loaded.size(); ++i)
            delete textures_loaded[i];
        //if (m_Behaviour)
            //delete m_Behaviour;
    }

private:

    void Load_Model(const std::string& fileDir);

    void Process_Node(aiNode* node, const aiScene* scene);

    void Process_Mesh(aiMesh* mesh, const aiScene* scene,
        std::vector<Vertex> &vertices,
        std::vector<size_t> &indices,
        std::vector<Texture*> &textures);

    std::vector<Texture*> Load_Material_Textures(aiMaterial* mat, aiTextureType type, std::string typeName);

};


