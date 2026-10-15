#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Matrix.hpp"
#include "Common/Vector.hpp"
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
        virtual void drawHighlight(Common::SharedPtr<Shader> shader, IdxType faceIndex) override;
        virtual Common::BBox<Scalar, 3> getBounds() const override;
        virtual IdxType touch(const PickContext &context) override;

        void setNodes(Common::DynamicArray<CoordType> &&);
        void setIndices(Common::DynamicArray<IdxType> &&);
        void setNormals(Common::DynamicArray<CoordType> &&);

        // Set per-triangle colors. Each element is a vec4 (RGBA).
        // If not called, colors default to gray in setup().
        void setColors(Common::DynamicArray<Common::Vector4f> &&colors);
        void setColor(const Common::Vector4f &color);
        void setHighlightScale(float scale);

    private:
        void uploadColors();
        void updateHighlightScaleMatrix();

        Common::DynamicArray<CoordType> _vertices;
        Common::DynamicArray<CoordType> _normals;
        Common::DynamicArray<IdxType> _indices;

        Common::SharedPtr<VertexArray> _vertexArray;
        Common::SharedPtr<VertexArray> _normalVertexArray;
        Common::SharedPtr<VertexBuffer> _vertexBuffer;
        Common::SharedPtr<VertexBuffer> _normalVertexBuffer;
        Common::SharedPtr<IndexBuffer> _indexBuffer;

        // Per-triangle color table stored in a Shader Storage Buffer Object (SSBO).
        // Each triangle has a vec4 color (RGBA). Default is gray when not provided.
        Common::DynamicArray<Common::Vector4f> _colors;
        unsigned int _colorSSBO{0};
        float _highlightScale{1.0f};
        bool _highlightScaleEnabled{false};
        Common::Matrix4f _highlightScaleMatrix{Common::Matrix4f::Identity()};
        // SSBO binding point used by shaders. Shaders should declare:
        // layout(std430, binding = 2) buffer ColorTable { vec4 colors[]; };
        static constexpr unsigned int COLOR_SSBO_BINDING = 2;
    };
} // namespace EgLab::RE