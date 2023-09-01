#pragma once
#include <vector>
#include<GL/glew.h>

#include "Log.h"

struct VertexBufferElement
{
	unsigned int type;
	unsigned int count;
	unsigned char normalized;
    VertexBuffer* VBO;

	static unsigned int GetSizeOfType(unsigned int type)
	{
		switch (type)
		{
		case GL_FLOAT:			return 4;
		case GL_UNSIGNED_INT:	return 4;
		case GL_UNSIGNED_BYTE:	return 1;
		}
		ASSERT(false);
		return 0;
	}
};

class VertexBufferLayout
{
private:
	std::vector<VertexBufferElement> m_Elements;

public:
	VertexBufferLayout(){}

	template<typename T>
    // count is the number of vbo's dimension, vbo a pointer what you want to push
	void Push(unsigned int count, VertexBuffer *vbo)
	{
		static_assert(false);
	}

	template<>
	void Push<float>(unsigned int count, VertexBuffer *vbo)
	{
		m_Elements.push_back({ GL_FLOAT, count ,GL_FALSE , vbo});
	}
	template<>
	void Push<unsigned int>(unsigned int count, VertexBuffer *vbo)
	{
		m_Elements.push_back({ GL_UNSIGNED_INT, count ,GL_FALSE , vbo});
	}
	template<>
	void Push<unsigned char>(unsigned int count, VertexBuffer *vbo)
	{
		m_Elements.push_back({ GL_UNSIGNED_BYTE, count ,GL_TRUE , vbo});
	}
	inline const std::vector<VertexBufferElement> GetElement() const { return m_Elements; }
	//inline unsigned int GetStride() const { return m_Stride; }

};
