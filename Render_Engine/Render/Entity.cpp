#include<iostream>

#include "Entity.h"
#include "Renderer.h"

#include "stb_image/stb_image.h"



Entity::Entity(const std::string& Shader_filepath)
{
    Transform transform;
    m_Transform = transform;
    m_Shader = new Shader(Shader_filepath);
    m_Shader->Bind();
}

Entity::Entity(Eigen::Vector3f position, Eigen::Vector3f rotation, Eigen::Vector3f scale)
{
    Transform transform(position,rotation,scale);
    m_Transform = transform;
    m_Shader = new Shader("res/shaders/Basic.shader");
    m_Shader->Bind();

    // set shader
    //shader.SetUniform3f("light.Position", LightPosition);
    
	//m_Shader->SetUniform1i("material.diffuse", 0);
	m_Shader->SetUniform1i("material.texture_specular1", 1);
    m_Shader->SetUniform1i("material.texture_diffuse1", 1);
    m_Shader->SetUniform1f("material.shininess", 32.0f);

}

Entity::Entity(Eigen::Vector3f position, Eigen::Vector3f rotation, Eigen::Vector3f scale, std::string ShaderPath)
{
    Transform transform(position,rotation,scale);
    m_Transform = transform;
    m_Shader = new Shader(ShaderPath);
    m_Shader->Bind();
}

void Entity::Load(const std::string& fileDir)
{
    Load_Model(fileDir);
    if(m_Behaviour)
        m_Behaviour->Load_Mesh(m_Meshes);
}



void Entity::Draw()
{
    for (size_t i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i]->Draw(m_Shader,m_Transform);
    }
}

void Entity::Start()
{
    if(m_Behaviour)
        m_Behaviour->Start();
}

void Entity::Update(float deltaTime)
{
    if(m_Behaviour)
        m_Behaviour->Update(deltaTime);
}

void Entity::Set_Window(GLFWwindow* window)
{
    m_Window = window;
    if(m_Behaviour)
        m_Behaviour->Set_Window(window);
}

// load camera parameter to shader
void Entity::Shader_Load_Camera(Eigen::Matrix4f &proj, Eigen::Matrix4f &view)
{
    m_Shader->Bind();
    
    m_Shader->SetUniformMat4f("u_MVP", proj);
    m_Shader->SetUniformMat4f("view", view);

    m_Shader->Unbind();

}



void Entity::Load_Model(const std::string& fileDir)
{
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(fileDir, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    m_fileDir = fileDir.substr(0, fileDir.find_last_of('/'));

    Process_Node(scene->mRootNode, scene);
}

void Entity::Process_Node(aiNode* node, const aiScene* scene)
{
    // deal with all of mesh nodes
    for (size_t i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* aimesh = scene->mMeshes[node->mMeshes[i]];

        std::vector<Eigen::Vector3f> vertices;
        std::vector<Eigen::Vector3f> normals;
        std::vector<Eigen::Vector2f> texcoords;
        std::vector<size_t> indices;
        std::vector<Texture*> textures;
        Process_Mesh(aimesh, scene, vertices, normals, texcoords, indices, textures);

        Mesh* mesh = new Mesh(vertices, normals, texcoords, indices, textures);

        m_Meshes.emplace_back(mesh);
    }

    // recursion all nodes
    for (size_t i = 0; i < node->mNumChildren; ++i)
        Process_Node(node->mChildren[i], scene);
}

void Entity::Process_Mesh(aiMesh* mesh, const aiScene* scene,
    std::vector<Eigen::Vector3f> &vertices,
    std::vector<Eigen::Vector3f> &normals,
    std::vector<Eigen::Vector2f> &texcoords,
    std::vector<size_t> &indices,
    std::vector<Texture*> &textures
    )
{
    
    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
        //Vertex vertex;

        // TODO: process position\normal\texture
        Eigen::Vector3f vector;
        vector.x() = mesh->mVertices[i].x;
        vector.y() = mesh->mVertices[i].y;
        vector.z() = mesh->mVertices[i].z;
        vertices.push_back(vector);

        vector.x() = mesh->mNormals[i].x;
        vector.y() = mesh->mNormals[i].y;
        vector.z() = mesh->mNormals[i].z;
        normals.push_back(vector);;

        if (mesh->mTextureCoords[0])
        {
            Eigen::Vector2f vec;
            vec.x() = mesh->mTextureCoords[0][i].x;
            vec.y() = mesh->mTextureCoords[0][i].y;
            texcoords.push_back(vec);
        }
        else
        {
            texcoords.push_back(Eigen::Vector2f(0.0f, 0.0f));
        }

    }

    // TODO: process index
    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    // process material
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // 1. diffuse maps
        std::vector<Texture*> diffuseMaps = Load_Material_Textures(material,aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        std::vector<Texture*> specularMaps = Load_Material_Textures(material,aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<Texture*> normalMaps = Load_Material_Textures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<Texture*> heightMaps = Load_Material_Textures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    }

}

std::vector<Texture*> Entity::Load_Material_Textures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture*> textures;
    for(size_t i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for(size_t j = 0; j < textures_loaded.size(); j++)
        {
            if(std::strcmp(textures_loaded[j]->GetFilePath().c_str(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true; 
                break;
            }
        }
        if(!skip)
        {   // 如果纹理还没有被加载，则加载它
            Texture *texture = new Texture(m_fileDir+"/" + str.C_Str(), typeName);
            textures.push_back(texture);
            textures_loaded.push_back(texture); // 添加到已加载的纹理中
        }
    }
    return textures;

}

