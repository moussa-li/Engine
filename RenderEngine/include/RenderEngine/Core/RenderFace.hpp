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

        /// @brief Draws many highlighted faces with one draw call per consecutive index run.
        /// @param shader Shader used for highlighting.
        /// @param indices Face indices to highlight.
        virtual void drawHighlightBatch(Common::SharedPtr<Shader> shader,
                                        const Common::DynamicArray<IdxType> &indices) override;
        /// @brief Returns the axis-aligned bounds of the face data.
        virtual Common::BBox<Scalar, 3> getBounds() const override;
        /// @brief Tests the face data against a picking context.
        /// @param context Screen-space picking parameters and camera matrices.
        /// @return Index of the picked face, or the invalid index when nothing is hit.
        virtual IdxType touch(const PickContext &context) override;

        /// @brief Collects the faces covered by a rubber-band rectangle.
        /// @param context Screen-space rectangle and camera matrices.
        /// @param outIndices Receives the index of every matching face.
        virtual void touchRect(const RectPickContext &context,
                               Common::DynamicArray<IdxType> &outIndices) const override;

        /// @brief Picking a face primitive yields face entities.
        PickKind getPickKind() const override
        {
            return PickKind::Face;
        }

        /// @brief Replaces the face vertex positions.
        void setNodes(Common::DynamicArray<CoordType> &&);
        /// @brief Replaces the face element indices.
        void setIndices(Common::DynamicArray<IdxType> &&);
        /// @brief Replaces the face vertex normals.
        void setNormals(Common::DynamicArray<CoordType> &&);

        /// @brief Sets one surface UV per face vertex (array-mode, 3 entries per triangle).
        /// @details The UVs are fed to the dedicated surface-UV shader, which interpolates them
        ///          across each triangle to colour the face.
        /// @param uvs UV coordinates matching the vertex array size.
        void setUvs(Common::DynamicArray<Common::Vector2f> &&uvs);

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
        /// @brief Enables zebra (isophote) stripe analysis shading.
        /// @details The lit color is replaced by bands reflected off the curved surface, which
        ///          reveals curvature discontinuities. Kept per primitive because the zebra
        ///          shader program is shared by every geometry face.
        /// @param enabled True to draw zebra stripes instead of the shaded color.
        /// @param frequency Stripe density; larger values give more, tighter bands.
        /// @param lightDirection Direction towards the virtual zebra light in model space. The
        ///        stripes mirror it instead of the camera eye ray, so the pattern does not move
        ///        when the camera does.
        /// @param darkColor Color of the dark bands; the light bands stay white.
        void setZebraStripes(bool enabled, float frequency, const Common::Vector3f &lightDirection,
                             const Common::Vector4f &darkColor);
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
        Common::DynamicArray<Common::Vector2f> _uvs;
        Common::DynamicArray<IdxType> _indices;
        // True when the normals were supplied externally and must not be recomputed.
        bool _normalsProvided{false};

        Common::SharedPtr<VertexArray> _vertexArray;
        Common::SharedPtr<VertexArray> _arrayVertexArray;
        Common::SharedPtr<VertexArray> _normalVertexArray;
        Common::SharedPtr<VertexBuffer> _vertexBuffer;
        Common::SharedPtr<VertexBuffer> _normalVertexBuffer;
        Common::SharedPtr<VertexBuffer> _arrayVertexBuffer;
        Common::SharedPtr<VertexBuffer> _arrayNormalVertexBuffer;
        Common::SharedPtr<VertexBuffer> _arrayCentroidVertexBuffer;
        Common::SharedPtr<VertexBuffer> _arrayUvVertexBuffer;
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
        bool _zebraEnabled{false};
        float _zebraFrequency{8.0f};
        Common::Vector3f _zebraLightDirection{0.0f, 0.0f, 1.0f};
        Common::Vector4f _zebraDarkColor{0.05f, 0.05f, 0.05f, 1.0f};
        DrawMode _drawMode{DrawMode::Arrays};
        Common::Matrix4f _highlightScaleMatrix{Common::Matrix4f::Identity()};
        // SSBO binding point used by shaders. Shaders should declare:
        // layout(std430, binding = 2) buffer ColorTable { vec4 colors[]; };
        static constexpr unsigned int COLOR_SSBO_BINDING = 2;
    };
} // namespace EgLab::RE