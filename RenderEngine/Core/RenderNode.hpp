#pragma once
#include "Common/DynamicArray.hpp"
#include "Core/Definites.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"
#include "RenderEngine/Core/VertexArray.hpp"
#include "RenderEngine/Core/VertexBuffer.hpp"

namespace EgLab::RE
{
    class Shader;
    class Transform;
    class Texture;
    class VertexArray;
    class ShaderStorageBuffer;

    class RenderEngineAPI RenderNode : public RenderPrimitive
    {
    public:
        virtual ~RenderNode();
        virtual void setup() override;
        virtual void draw(Common::SharedPtr<Shader> shader) override;
        virtual void drawHighlight(Common::SharedPtr<Shader> shader, IdxType nodeIndex) override;
        virtual Common::BBox<Scalar, 3> getBounds() const override;

        void setNodes(Common::DynamicArray<CoordType> &&);
        void setColor(const Common::Vector4f &color);

        virtual IdxType touch(const PickContext &context) override;

    private:
        Common::DynamicArray<CoordType> _vertices;
        Common::Vector4f _color{0.18f, 0.24f, 0.30f, 1.0f};
        Common::SharedPtr<VertexArray> _vertexArray;
        Common::SharedPtr<VertexBuffer> _vertexBuffer;
    };
} // namespace EgLab::RE