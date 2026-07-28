#pragma once
#include "Common/DynamicArray.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/IndexBuffer.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"
#include "RenderEngine/Core/VertexArray.hpp"


namespace EgLab
{
    class Shader;
    class Transform;
    class Texture;
    class VertexArray;
    class VertexBuffer;
    class ShaderStorageBuffer;
    class IndexBuffer;

    class RenderLine : public RenderPrimitive
    {
    public:
        virtual ~RenderLine() {};
        virtual void setup() override;
        virtual void draw(SharedPtr<Shader> shader) override;

        void setNodes(DynamicArray<CoordType>&&);
        void setIndices(DynamicArray<IdxType>&&);

    private:
        DynamicArray<CoordType> _vertices;
        DynamicArray<IdxType> _indices;

        SharedPtr<VertexArray> _vertexArray;
        SharedPtr<VertexBuffer> _vertexBuffer;
        SharedPtr<IndexBuffer> _indexBuffer;
    };
} // namespace EgLab