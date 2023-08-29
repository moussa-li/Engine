#include "Texture.h"
#include "Log.h"

#include <gl/glew.h>

#include <vector>

#include "stb_image/stb_image.h"

Texture::Texture(const std::string& path, std::string type)
	: m_FilePath(path), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0), m_Type(type)
{
    GLCall(glGenTextures(1, &m_RendererId));
    if (type == "skybox")
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererId);
        int width, height, nrChannels;
        std::vector<std::string> faces = { 
            "right.jpg",
            "left.jpg",
            "top.jpg",
            "bottom.jpg",
            "front.jpg",
            "back.jpg" };
        for (int i = 0; i < 6; ++i)
        {
            unsigned char *data = stbi_load((path+"/"+faces[i]).c_str(), &width, &height, &nrChannels, 0);
            stbi_set_flip_vertically_on_load(true);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                             0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
                );
                stbi_image_free(data);
            }
            else
            {
                std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
                stbi_image_free(data);
            }

        }
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    }
    else 
    {
    
	    stbi_set_flip_vertically_on_load(1);
    	m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);
    
        if (m_LocalBuffer)
        {
    
            GLenum format;
            if (m_BPP == 1)
                format = GL_RED;
            else if (m_BPP == 3)
                format = GL_RGB;
            else if (m_BPP == 4)
                format = GL_RGBA;
    
            GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererId));
        
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        
            GLCall(glTexImage2D(GL_TEXTURE_2D, 0, format, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, m_LocalBuffer));
            GLCall(glGenerateMipmap(GL_TEXTURE_2D));
            GLCall(glBindTexture(GL_TEXTURE_2D, 0));
        }
        else
        {
            Log::Log(Log::Error, "Texture failed to load at path " + path);
    		stbi_image_free(m_LocalBuffer);
        }
    }
}

Texture::~Texture()
{
	GLCall(glDeleteTextures(1, &m_RendererId));
}

void Texture::Bind(unsigned int slot /*= 0*/) const
{
    if (m_Type == "skybox")
    {
	    GLCall(glActiveTexture(GL_TEXTURE0 + slot));
        GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererId));
    }
    else 
    {
	    GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	    GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererId));
    }
}

void Texture::Unibind() const
{
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

}
