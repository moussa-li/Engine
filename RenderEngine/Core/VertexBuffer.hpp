#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Object.hpp"
#include "Common/Vector.hpp"


namespace EgLab
{
    class VertexBuffer : public Object
    {
    public:
        VertexBuffer(const void* data, size_t size);
        VertexBuffer(DynamicArray<Vector3f>& v);
        VertexBuffer(DynamicArray<Vector2f>& v);
        ~VertexBuffer();

        void bind() const;
        void unbind() const;

    private:
        class Impl;
        Impl* _impl;
    };
} // namespace EgLab