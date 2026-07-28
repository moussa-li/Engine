#pragma once
#include "Common/DynamicArray.hpp"
#include "RenderEngine/Core/Definites.hpp"
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

    class RenderNode : public RenderPrimitive
    {
    public:
        virtual ~RenderNode() {};
        virtual void setup() override;
        virtual void draw(SharedPtr<Shader> shader) override;

        void setNodes(DynamicArray<CoordType>&&);

    private:
        DynamicArray<CoordType> _vertices;
        SharedPtr<VertexArray> _vertexArray;
        SharedPtr<VertexBuffer> _vertexBuffer;
    };
} // namespace EgLab