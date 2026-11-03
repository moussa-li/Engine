#pragma once

#include "RenderEngine/Core/RenderEngineAPI.hpp"
#include "VertexBufferLayout.hpp"

namespace EgLab::RE
{
    /// OpenGL vertex array object that records vertex attribute bindings.
    class RenderEngineAPI VertexArray
    {
    public:
        /// @brief Creates a vertex array object.
        VertexArray();
        /// @brief Releases the vertex array object.
        ~VertexArray();

        /// @brief Adds the vertex buffers described by a layout.
        /// @param layout Vertex attribute layout to bind.
        void addBuffer(const VertexBufferLayout& layout);

        /// @brief Binds this vertex array object.
        void bind() const;
        /// @brief Unbinds the current vertex array object.
        void unBind() const;

    private:
        class Impl;
        Impl* _impl;
    };

} // namespace EgLab::RE