#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "VertexBuffer.hpp"

namespace EgLab
{
    enum class BufferType
    {
        FLOAT,
        DOUBLE
    };

    struct VertexBufferElement
    {
        BufferType type;
        size_t count;
        bool normalized;
        SharedPtr<VertexBuffer> VBO;
    };

    class VertexBufferLayout
    {
    public:
        VertexBufferLayout() = default;

        template <typename T>
        void pushBack(unsigned int count, SharedPtr<VertexBuffer> vbo)
        {
            static_assert(false);
        }

        template <>
        void pushBack<float>(unsigned int count, SharedPtr<VertexBuffer> vbo)
        {
            _elements.pushBack({BufferType::FLOAT, count, false, vbo});
        }

        inline const DynamicArray<VertexBufferElement> getElement() const
        {
            return _elements;
        }

    private:
        DynamicArray<VertexBufferElement> _elements;
    };

} // namespace EgLab
