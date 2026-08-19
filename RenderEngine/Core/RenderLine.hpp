#pragma once
#include "Common/DynamicArray.hpp"
#include "Core/Definites.hpp"
#include "RenderEngine/Core/IndexBuffer.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"
#include "RenderEngine/Core/VertexArray.hpp"

namespace EgLab::RE
{
    class Shader;
    class Transform;
    class Texture;
    class VertexArray;
    class VertexBuffer;
    class ShaderStorageBuffer;
    class IndexBuffer;

    class RenderEngineAPI RenderLine : public RenderPrimitive
    {
    public:
        virtual ~RenderLine();
        virtual void setup() override;
        virtual void draw(Common::SharedPtr<Shader> shader) override;

        void setNodes(Common::DynamicArray<CoordType>&&);
        void setIndices(Common::DynamicArray<IdxType>&&);

    private:
        Common::DynamicArray<CoordType> _vertices;
        Common::DynamicArray<IdxType> _indices;

        Common::SharedPtr<VertexArray> _vertexArray;
        Common::SharedPtr<VertexBuffer> _vertexBuffer;
        Common::SharedPtr<IndexBuffer> _indexBuffer;
    };
} // namespace EgLab::RE