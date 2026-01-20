#pragma once

#include "VertexBufferLayout.hpp"

namespace EgLab
{
    class VertexArray
    {
    public:
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