#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Matrix.hpp"
#include "Common/Vector.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/IndexBuffer.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"
#include "RenderEngine/Core/VertexArray.hpp"

namespace EgLab::RE
{
    /// Renderable surface primitive supporting indexed or array drawing and picking.
    class RenderEngineAPI RenderFace : public RenderPrimitive
    {
    public:
        /// Selects whether face vertices are drawn from arrays or an index buffer.
        enum class DrawMode
        {
            /// Draw vertices as a sequential array.
            Arrays,
            /// Draw vertices using element indices.
            Elements
        };

        /// @brief Releases face rendering resources.
        virtual ~RenderFace();
        /// @brief Creates or updates GPU resources for the face data.
        virtual void setup() override;
        /// @brief Draws the face using the specified shader.
        /// @param shader Shader used for rendering.
        virtual void draw(Common::SharedPtr<Shader> shader) override;
        /// @brief Draws the specified face with its highlight effect.
        /// @param shader Shader used for highlighting.
        /// @param faceIndex Index of the face to highlight.
        virtual void drawHighlight(Common::SharedPtr<Shader> shader, IdxType faceIndex) override;
        /// @brief Returns the axis-aligned bounds of the face data.
        virtual Common::BBox<Scalar, 3> getBounds() const override;
        /// @brief Tests the face data against a picking context.
        /// @param context Screen-space picking parameters and camera matrices.
        /// @return Index of the picked face, or the invalid index when nothing is hit.
        virtual IdxType touch(const PickContext &context) override;

        /// @brief Replaces the face vertex positions.
        void setNodes(Common::DynamicArray<CoordType> &&);
        /// @brief Replaces the face element indices.
        void setIndices(Common::DynamicArray<IdxType> &&);
        /// @brief Replaces the face vertex normals.
        void setNormals(Common::DynamicArray<CoordType> &&);

        /** @brief Sets one RGBA color per triangle.
         * @param colors Triangle colors; gray is used when no colors are provided.
         */
        void setColors(Common::DynamicArray<Common::Vector4f> &&colors);
        /// @brief Sets a uniform RGBA color for the faces.
        /// @param color Color applied to the faces.
        void setColor(const Common::Vector4f &color);
        /// @brief Sets the scale applied to highlighted faces.
        /// @param scale Highlight scale factor.
        void setHighlightScale(float scale);
        /// @brief Sets how far each face shrinks toward its own centroid.
        /// @param shrink Shrink amount in [0, 1]; 0 keeps the full face, 1 collapses to the
        /// centroid.
        void setFaceShrink(float shrink);
        /// @brief Sets per-vertex shrink centers (element-face centers) for array-mode drawing.
        /// @param centers Three entries per triangle, all equal to that element face's center.
        void setFaceCenters(Common::DynamicArray<CoordType> &&centers);
        /// @brief Sets the depth bias used when drawing faces.
        /// @param factor Slope-scaled depth bias factor.
        /// @param units Constant depth bias units.
        void setDepthOffset(float factor, float units);
        /// @brief Selects the face drawing mode.
        /// @param mode Array or indexed drawing mode.
        void setDrawMode(DrawMode mode);
        /// @brief Returns the current face drawing mode.
        DrawMode getDrawMode() const;

    private:
        void uploadColors();
        void updateHighlightScaleMatrix();

        Common::DynamicArray<CoordType> _vertices;
        Common::DynamicArray<CoordType> _normals;
        Common::DynamicArray<IdxType> _indices;

        Common::SharedPtr<VertexArray> _vertexArray;
        Common::SharedPtr<VertexArray> _arrayVertexArray;
        Common::SharedPtr<VertexArray> _normalVertexArray;
        Common::SharedPtr<VertexBuffer> _vertexBuffer;
        Common::SharedPtr<VertexBuffer> _normalVertexBuffer;
        Common::SharedPtr<VertexBuffer> _arrayVertexBuffer;
        Common::SharedPtr<VertexBuffer> _arrayNormalVertexBuffer;
        Common::SharedPtr<VertexBuffer> _arrayCentroidVertexBuffer;
        Common::SharedPtr<IndexBuffer> _indexBuffer;

        // Per-triangle color table stored in a Shader Storage Buffer Object (SSBO).
        // Each triangle has a vec4 color (RGBA). Default is gray when not provided.
        Common::DynamicArray<Common::Vector4f> _colors;
        unsigned int _colorSSBO{0};
        float _highlightScale{1.0f};
        bool _highlightScaleEnabled{false};
        float _faceShrink{0.0f};
        Common::DynamicArray<CoordType> _faceCenters;
        float _depthOffsetFactor{1.0f};
        float _depthOffsetUnits{1.0f};
        DrawMode _drawMode{DrawMode::Arrays};
        Common::Matrix4f _highlightScaleMatrix{Common::Matrix4f::Identity()};
        // SSBO binding point used by shaders. Shaders should declare:
        // layout(std430, binding = 2) buffer ColorTable { vec4 colors[]; };
        static constexpr unsigned int COLOR_SSBO_BINDING = 2;
    };
} // namespace EgLab::RE