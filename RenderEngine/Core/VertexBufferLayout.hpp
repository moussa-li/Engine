#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "VertexBuffer.hpp"

namespace EgLab::RE
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
        Common::SharedPtr<VertexBuffer> VBO;
    };

    class VertexBufferLayout
    {
    public:
        VertexBufferLayout() = default;

        template <typename T>
        void pushBack(unsigned int count, Common::SharedPtr<VertexBuffer> vbo)
        {
            static_assert(false);
        }

        template <>
        void pushBack<float>(unsigned int count, Common::SharedPtr<VertexBuffer> vbo)
        {
            _elements.pushBack({BufferType::FLOAT, count, false, vbo});
        }

        inline const Common::DynamicArray<VertexBufferElement> getElement() const
        {
            return _elements;
        }

    private:
        Common::DynamicArray<VertexBufferElement> _elements;
    };

} // namespace EgLab::RE
