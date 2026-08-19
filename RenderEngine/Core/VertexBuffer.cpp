#include "VertexBuffer.hpp"

#include <GL/glew.h>

#include "Definites.hpp"
#include "GLWrapper.hpp"

namespace EgLab::RE
{
    class VertexBuffer::Impl
    {
    public:
        IdType getId() const
        {
            return static_cast<IdType>(_renderId);
        }

        void genBuffer(const void* data, unsigned int size)
        {
            GLCall(glGenBuffers(1, &_renderId));
            GLCall(glBindBuffer(GL_ARRAY_BUFFER, _renderId));
            GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
            _data = data;
            _size = size;
        }

        void bind() const
        {
            GLCall(glBindBuffer(GL_ARRAY_BUFFER, _renderId));
        }

        void unbind() const
        {
            GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
        }

    private:
        GLuint _renderId;

        const void* _data;
        size_t _size;
    };

    VertexBuffer::VertexBuffer(const void* data, size_t size) : _impl(new Impl)
    {
        _impl->genBuffer(data, size);
    }

    VertexBuffer::VertexBuffer(Common::DynamicArray<Common::Vector3f>& v) : _impl(new Impl)
    {
        _impl->genBuffer(&(v[0]), v.size() * sizeof(float) * 3);
    }
    VertexBuffer::VertexBuffer(Common::DynamicArray<Common::Vector2f>& v) : _impl(new Impl)
    {
        _impl->genBuffer(&(v[0]), v.size() * sizeof(float) * 2);
    }

    void VertexBuffer::bind() const
    {
        _impl->bind();
    }

    void VertexBuffer::unbind() const
    {
        _impl->unbind();
    }

    VertexBuffer::~VertexBuffer()
    {
        if (_impl != nullptr)
        {
            delete _impl;
            _impl = nullptr;
        }
    }

} // namespace EgLab::RE