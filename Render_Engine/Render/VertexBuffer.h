#pragma once

#include "Vertex.h"

#include "libexport.h"

class DLLAPI VertexBuffer
{
private:
	unsigned int m_RendererId;
public:
	VertexBuffer(const void* data, unsigned int size);
	VertexBuffer(std::vector<Vertex> vertices);
	~VertexBuffer();

	void Bind() const;
	void Unbind() const;
};
