#pragma once

#include "libexport.h"

class DLLAPI UniformBuffer
{
private:
	unsigned int m_RendererId;
    unsigned int m_BindingId;

    unsigned int m_Size;
    unsigned int m_Stride;
public:
    UniformBuffer(unsigned int size, unsigned int bindingId);

    void InputData(void *point, unsigned int size);

    void OutputData(void *point, unsigned int &size);


    void Bind();
    void UnBind();
};



