#include "VertexArray.hpp"

#include <GL/glew.h>

#include "GLWrapper.hpp"

namespace EgLab
{
    class VertexArray::Impl
    {
    public:
        Impl()
        {
            GLCall(glGenVertexArrays(1, &_renderId));
        }

        ~Impl()
        {
            GLCall(glDeleteVertexArrays(1, &_renderId));
        }

        unsigned int getGLType(BufferType bt)
        {
            switch (bt)
            {
            case EgLab::BufferType::FLOAT:
                return GL_FLOAT;
            case EgLab::BufferType::DOUBLE:
                return GL_DOUBLE;
            }
        }

        unsigned char getGLNorm(bool norm)
        {
            if (norm)
                return GL_TRUE;
            else
                return GL_FALSE;
        }

        void addBuffer(const VertexBufferLayout& layout)
        {
            bind();
            const auto& elements = layout.getElement();
            for (size_t i = 0; i < elements.size(); i++)
            {
                const auto& element = elements[i];
                element.VBO->bind();
                GLCall(glEnableVertexAttribArray(i));
                GLCall(glVertexAttribPointer(i, element.count, getGLType(element.type),
                                             getGLNorm(element.normalized), 0, NULL));
            }
        }

        void bind() const
        {
            GLCall(glBindVertexArray(_renderId));
        }

        void unbind() const
        {
            GLCall(glBindVertexArray(0));
        }

    private:
        GLuint _renderId;
    };

    VertexArray::VertexArray() : _impl(new Impl)
    {
    }

    void VertexArray::addBuffer(const VertexBufferLayout& layout)
    {
        _impl->addBuffer(layout);
    }

    void VertexArray::bind() const
    {
        _impl->bind();
    }

    void VertexArray::unBind() const
    {
        _impl->unbind();
    }

    VertexArray::~VertexArray()
    {
        if (_impl != nullptr)
        {
            delete _impl;
            _impl = nullptr;
        }
    }

} // namespace EgLab