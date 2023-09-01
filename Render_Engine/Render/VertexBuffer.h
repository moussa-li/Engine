#pragma once

#include "Vertex.h"

#include "libexport.h"

class DLLAPI VertexBuffer
{
private:
	unsigned int m_RendererId;
public:
    void* m_Data;
    unsigned int Size;
public:
	VertexBuffer(const void* data, unsigned int size);
	//VertexBuffer(std::vector<Vertex> vertices);
	VertexBuffer(std::vector<Eigen::Vector3f> *v);
	VertexBuffer(std::vector<Eigen::Vector2f> *v);
	~VertexBuffer();

	void Bind() const;
	void Unbind() const;
};
