#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "VertexBuffer.hpp"

namespace EgLab::RE
{
    /// Supported scalar storage types for vertex attributes.
    enum class BufferType
    {
        /// 32-bit floating-point values.
        FLOAT,
        /// 64-bit floating-point values.
        DOUBLE
    };

    /// Describes one vertex attribute buffer and its component count.
    struct VertexBufferElement
    {
        /// Scalar type stored in the attribute.
        BufferType type;
        /// Number of scalar components per attribute.
        size_t count;
        /// Whether values should be normalized when read by OpenGL.
        bool normalized;
        /// Buffer containing the attribute values.
        Common::SharedPtr<VertexBuffer> VBO;
    };

    /// Collects vertex attribute buffer descriptions for a vertex array.
    class VertexBufferLayout
    {
    public:
        /// @brief Creates an empty vertex buffer layout.
        VertexBufferLayout() = default;

        /// @brief Adds a typed vertex attribute buffer to the layout.
        /// @tparam T Attribute scalar type supported by a specialization.
        /// @param count Number of components in each attribute.
        /// @param vbo Buffer containing the attribute values.
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

        /// @brief Returns the configured vertex attributes.
        /// @return A copy of the layout elements.
        inline const Common::DynamicArray<VertexBufferElement> getElement() const
        {
            return _elements;
        }

    private:
        Common::DynamicArray<VertexBufferElement> _elements;
    };

} // namespace EgLab::RE
