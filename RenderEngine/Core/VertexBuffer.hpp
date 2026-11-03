#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Object.hpp"
#include "Common/Vector.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"

namespace EgLab::RE
{
    /// OpenGL vertex buffer that stores raw data or common vector attributes.
    class RenderEngineAPI VertexBuffer
    {
    public:
        /**
         * @brief Creates a vertex buffer from a raw memory range.
         * @param data Pointer to the source data.
         * @param size Size of the source data in bytes.
         */
        VertexBuffer(const void* data, size_t size);
        /// @brief Creates a vertex buffer from three-dimensional vectors.
        /// @param v Source vector data.
        VertexBuffer(Common::DynamicArray<Common::Vector3f>& v);
        /// @brief Creates a vertex buffer from two-dimensional vectors.
        /// @param v Source vector data.
        VertexBuffer(Common::DynamicArray<Common::Vector2f>& v);
        /// @brief Releases the vertex buffer.
        ~VertexBuffer();

        /// @brief Binds this vertex buffer.
        void bind() const;
        /// @brief Unbinds the current vertex buffer.
        void unbind() const;

    private:
        class Impl;
        Impl* _impl;
    };
} // namespace EgLab::RE