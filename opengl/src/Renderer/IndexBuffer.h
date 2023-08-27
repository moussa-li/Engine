#pragma once

#include <vector>

class IndexBuffer
{
private:
	unsigned int m_RendererId;
	unsigned int m_Count;
public:
	IndexBuffer(const unsigned int* data, unsigned int count);
	IndexBuffer(std::vector<size_t> indices);
	~IndexBuffer();

	void Bind() const;
	void Unbind() const;

	inline unsigned int getCount() const {
		return m_Count;
	}
};

