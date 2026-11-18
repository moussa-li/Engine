#pragma once
#include "Common/DynamicArray.hpp"
#include "Common/HashSet.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/IndexBuffer.hpp"
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

        /// @brief Highlights many nodes with a single indexed draw call.
        /// @param shader Shader used for highlighting.
        /// @param indices Node indices to highlight.
        virtual void drawHighlightBatch(Common::SharedPtr<Shader> shader,
                                        const Common::DynamicArray<IdxType> &indices) override;
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

        /// @brief Collects the nodes covered by a rubber-band rectangle.
        /// @param context Screen-space rectangle and camera matrices.
        /// @param outIndices Receives the indices of every matching node.
        virtual void touchRect(const RectPickContext &context,
                               Common::DynamicArray<IdxType> &outIndices) const override;

        /// @brief Marks nodes as hidden without changing the vertex index mapping.
        /// @details Hidden nodes are kept in the buffers so every external index stays valid;
        ///          the node shader degenerates them and picking skips them.
        /// @param hiddenNodeIndices Local node indices to hide.
        void setHiddenNodes(const Common::HashSet<IdxType> &hiddenNodeIndices);

        /// @brief Returns the number of nodes in the primitive.
        size_t getNodeCount() const
        {
            return _vertices.size();
        }

        /// @brief Picking a node primitive yields node entities.
        PickKind getPickKind() const override
        {
            return PickKind::Node;
        }

    private:
        void rebuildHiddenFlags();
        void uploadHiddenFlagsIfNeeded();

        Common::DynamicArray<CoordType> _vertices;
        Common::DynamicArray<float> _hiddenFlags;
        Common::HashSet<IdxType> _hiddenNodeIndices;
        Common::Vector4f _color{0.18f, 0.24f, 0.30f, 1.0f};
        float _pointSize{3.0f};
        bool _depthTestEnabled{true};
        bool _hiddenFlagsDirty{false};
        Common::SharedPtr<VertexArray> _vertexArray;
        Common::SharedPtr<VertexBuffer> _vertexBuffer;
        Common::SharedPtr<VertexBuffer> _hiddenVertexBuffer;
        // Index buffer backing drawHighlightBatch(); rebuilt only when the selection changes.
        Common::SharedPtr<IndexBuffer> _highlightIndexBuffer;
        Common::DynamicArray<IdxType> _highlightIndices;
    };
} // namespace EgLab::RE