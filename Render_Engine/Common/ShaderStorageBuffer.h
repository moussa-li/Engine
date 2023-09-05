#pragma once

#include "libexport.h"

class DLLAPI ShaderStorageBuffer
{
private:
	unsigned int m_RendererId;
    unsigned int m_BindingId;

    unsigned int m_Size;
    unsigned int m_Stride;
public:
    ShaderStorageBuffer(unsigned int size, unsigned int bindingId);

    ~ShaderStorageBuffer();

    void InputData(void *point, unsigned int size);

    void OutputData(void *point, unsigned int &size);

    void OutputData(void* point);
    void Bind();
    void UnBind();
};


