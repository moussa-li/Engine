#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Object.hpp"
#include "Definites.hpp"

namespace EgLab::RE
{
    /// OpenGL index buffer that stores element indices for indexed drawing.
    class IndexBuffer
    {
    public:
        /**
         * @brief Creates an index buffer from the supplied indices.
         * @param indeices Indices uploaded to the GPU.
         */
        IndexBuffer(const Common::DynamicArray<IdxType>& indeices);
        /// @brief Releases the underlying index buffer.
        ~IndexBuffer();

        /// @brief Binds this index buffer for drawing.
        void bind() const;
        /// @brief Unbinds the current index buffer.
        void unBind() const;

        /// @brief Returns the number of indices stored in the buffer.
        unsigned int getCount() const;

    private:
        class Impl;
        Impl* _impl;
    };

} // namespace EgLab::RE