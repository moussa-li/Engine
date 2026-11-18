#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"

namespace EgLab::RE
{
    class Shader;

    /// Scene primitive that draws the highlight of selected elements of another primitive.
    ///
    /// Picking does not draw anything by itself: the pick handler inserts one of these into
    /// the scene, so highlight output goes through the same render path as every other
    /// primitive. It is added to the highlight-shader bucket and replays the source
    /// primitive's drawHighlight() for each index.
    class RenderEngineAPI RenderPickHighlight : public RenderPrimitive
    {
    public:
        /// @brief Sets the primitive whose elements are highlighted.
        void setSource(Common::SharedPtr<RenderPrimitive> source);

        /// @brief Sets the shader the source primitive is drawn with.
        /// @details The highlight reuses the source shader because drawHighlight() writes
        ///          the primitive's own uniforms.
        void setSourceShader(Common::SharedPtr<Shader> shader);

        /// @brief Sets the element indices to highlight.
        void setIndices(Common::DynamicArray<IdxType> &&indices);

        /// @brief Reports whether there is anything to draw.
        bool isEmpty() const
        {
            return _source == nullptr || _indices.empty();
        }

        /// @brief Highlight drawing owns no GPU resources of its own.
        void setup() override
        {
        }

        /// @brief Draws every highlighted element of the source primitive.
        void draw(Common::SharedPtr<Shader> shader) override;

        /// @brief Returns the bounds of the source primitive.
        Common::BBox<Scalar, 3> getBounds() const override;

    private:
        Common::SharedPtr<RenderPrimitive> _source;
        Common::SharedPtr<Shader> _sourceShader;
        Common::DynamicArray<IdxType> _indices;
    };
} // namespace EgLab::RE
