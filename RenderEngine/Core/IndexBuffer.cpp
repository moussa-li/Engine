#include "IndexBuffer.hpp"

#include <GL/glew.h>

namespace EgLab
{
    class IndexBuffer::Impl
    {
    public:
        Impl(const DynamicArray<IdxType>& indeices)
        {
            glGenBuffers(1, &_rendererId);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rendererId);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indeices.size() * sizeof(IdxType),
                         indeices.data(), GL_STATIC_DRAW);
        }

        ~Impl()
        {
            glDeleteBuffers(1, &_rendererId);
        }

        void bind() const
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rendererId);
        }

        void unBind() const
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        IdType _rendererId;
        IdxType _count;
    };

    IndexBuffer::IndexBuffer(const DynamicArray<IdxType>& indeices) : _impl(new Impl(indeices))
    {
    }

    IndexBuffer::~IndexBuffer()
    {
        if (_impl)
        {
            delete _impl;
            _impl = nullptr;
        }
    }

    void IndexBuffer::bind() const
    {
        _impl->bind();
    }
    void IndexBuffer::unBind() const
    {
        _impl->unBind();
    }

    unsigned int IndexBuffer::getCount() const
    {
        return _impl->_count;
    }

} // namespace EgLab