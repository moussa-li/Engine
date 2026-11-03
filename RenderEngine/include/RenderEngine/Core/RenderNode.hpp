#pragma once
#include "Common/DynamicArray.hpp"
#include "RenderEngine/Core/Definites.hpp"
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

    /// Renderable point primitive supporting highlighting and picking.
    class RenderEngineAPI RenderNode : public RenderPrimitive
    {
    public:
        /// @brief Releases node rendering resources.
        virtual ~RenderNode();
        /// @brief Creates or updates GPU resources for the node data.
        virtual void setup() override;
        /// @brief Draws the nodes using the specified shader.
        /// @param shader Shader used for rendering.
        virtual void draw(Common::SharedPtr<Shader> shader) override;
        /// @brief Highlights the specified node.
        /// @param shader Shader used for highlighting.
        /// @param nodeIndex Index of the node to highlight.
        virtual void drawHighlight(Common::SharedPtr<Shader> shader, IdxType nodeIndex) override;
        /// @brief Returns the axis-aligned bounds of the node data.
        virtual Common::BBox<Scalar, 3> getBounds() const override;

        /// @brief Replaces node positions with the supplied data.
        void setNodes(Common::DynamicArray<CoordType> &&);
        /// @brief Sets the node color.
        /// @param color RGBA node color.
        void setColor(const Common::Vector4f &color);
        /// @brief Sets the rendered point size.
        /// @param pointSize Point diameter in pixels.
        void setPointSize(float pointSize);
        /// @brief Enables or disables depth testing for the nodes.
        /// @param enabled Whether depth testing is enabled.
        void setDepthTestEnabled(bool enabled);

        /// @brief Tests the node data against a picking context.
        /// @param context Screen-space picking parameters and camera matrices.
        /// @return Index of the picked node, or the invalid index when nothing is hit.
        virtual IdxType touch(const PickContext &context) override;

    private:
        Common::DynamicArray<CoordType> _vertices;
        Common::Vector4f _color{0.18f, 0.24f, 0.30f, 1.0f};
        float _pointSize{3.0f};
        bool _depthTestEnabled{true};
        Common::SharedPtr<VertexArray> _vertexArray;
        Common::SharedPtr<VertexBuffer> _vertexBuffer;
    };
} // namespace EgLab::RE