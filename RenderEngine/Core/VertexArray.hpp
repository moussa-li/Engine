#pragma once

#include "RenderEngine/Core/RenderEngineAPI.hpp"
#include "VertexBufferLayout.hpp"

namespace EgLab::RE
{
    class RenderEngineAPI VertexArray
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

} // namespace EgLab::RE