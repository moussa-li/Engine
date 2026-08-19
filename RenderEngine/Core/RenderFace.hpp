#pragma once

#include "Common/DynamicArray.hpp"
#include "Core/Definites.hpp"
#include "RenderEngine/Core/IndexBuffer.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"
#include "RenderEngine/Core/VertexArray.hpp"

namespace EgLab::RE
{
    class RenderEngineAPI RenderFace : public RenderPrimitive
    {
    public:
        virtual ~RenderFace();
        virtual void setup() override;
        virtual void draw(Common::SharedPtr<Shader> shader) override;

        void setNodes(Common::DynamicArray<CoordType> &&);
        void setIndices(Common::DynamicArray<IdxType> &&);
        void setNormals(Common::DynamicArray<CoordType> &&);

    private:
        Common::DynamicArray<CoordType> _vertices;
        Common::DynamicArray<CoordType> _normals;
        Common::DynamicArray<IdxType> _indices;

        Common::SharedPtr<VertexArray> _vertexArray;
        Common::SharedPtr<VertexArray> _normalVertexArray;
        Common::SharedPtr<VertexBuffer> _vertexBuffer;
        Common::SharedPtr<VertexBuffer> _normalVertexBuffer;
        Common::SharedPtr<IndexBuffer> _indexBuffer;
    };
} // namespace EgLab::RE