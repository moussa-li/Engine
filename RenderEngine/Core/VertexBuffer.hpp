#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Object.hpp"
#include "Common/Vector.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"


namespace EgLab::RE
{
    class RenderEngineAPI VertexBuffer
    {
    public:
        VertexBuffer(const void* data, size_t size);
        VertexBuffer(Common::DynamicArray<Common::Vector3f>& v);
        VertexBuffer(Common::DynamicArray<Common::Vector2f>& v);
        ~VertexBuffer();

        void bind() const;
        void unbind() const;

    private:
        class Impl;
        Impl* _impl;
    };
} // namespace EgLab::RE