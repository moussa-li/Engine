#pragma once
#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "RenderEngine/Core/Definites.hpp"
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

    /// Renderable line primitive supporting indexed drawing and picking.
    class RenderEngineAPI RenderLine : public RenderPrimitive
    {
    public:
        /// @brief Releases line rendering resources.
        virtual ~RenderLine();
        /// @brief Creates or updates GPU resources for the line data.
        virtual void setup() override;
        /// @brief Draws the line using the specified shader.
        /// @param shader Shader used for rendering.
        virtual void draw(Common::SharedPtr<Shader> shader) override;
        /// @brief Draws the specified line with its highlight effect.
        /// @param shader Shader used for highlighting.
        /// @param lineIndex Index of the line to highlight.
        virtual void drawHighlight(Common::SharedPtr<Shader> shader, IdxType lineIndex) override;
        /// @brief Returns the axis-aligned bounds of the line data.
        virtual Common::BBox<Scalar, 3> getBounds() const override;
        /// @brief Tests the line data against a picking context.
        /// @param context Screen-space picking parameters and camera matrices.
        /// @return Index of the picked line, or the invalid index when nothing is hit.
        virtual IdxType touch(const PickContext& context) override;

        /// @brief Replaces line vertex positions with the supplied data.
        void setNodes(Common::DynamicArray<CoordType>&&);
        /// @brief Replaces line endpoint indices with the supplied data.
        void setIndices(Common::DynamicArray<IdxType>&&);
        /// @brief Sets the line color.
        /// @param color RGBA line color.
        void setColor(const Common::Vector4f& color);
        /// @brief Sets the rendered line width.
        /// @param width Line width in pixels.
        void setLineWidth(float width);

    private:
        Common::DynamicArray<CoordType> _vertices;
        Common::DynamicArray<IdxType> _indices;
        Common::Vector4f _color;
        float _lineWidth{2.0f};
        bool _hasColor{false};

        Common::SharedPtr<VertexArray> _vertexArray;
        Common::SharedPtr<VertexBuffer> _vertexBuffer;
        Common::SharedPtr<IndexBuffer> _indexBuffer;
    };
} // namespace EgLab::RE