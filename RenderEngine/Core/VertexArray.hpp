#pragma once

#include "VertexBufferLayout.hpp"

namespace EgLab
{
    class VertexArray
    {
        VertexArray();
        ~VertexArray();

        void addBuffer(const VertexBufferLayout& layout);

        void bind() const;
        void unBind() const;

    private:
        class Impl;
        Impl* _impl;
    };

} // namespace EgLab