#include "Texture.h"
#include "Log.h"

#include <gl/glew.h>

#include "stb_image/stb_image.h"

Texture::Texture(const std::string& path, std::string type)
	: m_FilePath(path), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0), m_Type(type)
{
    GLCall(glGenTextures(1, &m_RendererId));
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

Texture::~Texture()
{
	GLCall(glDeleteTextures(1, &m_RendererId));
}

void Texture::Bind(unsigned int slot /*= 0*/) const
{
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererId));
}

void Texture::Unibind() const
{
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

}
