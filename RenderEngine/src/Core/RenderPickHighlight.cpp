#include "RenderEngine/Core/RenderPickHighlight.hpp"

#include "Common/Utils.hpp"

namespace EgLab::RE
{
    void RenderPickHighlight::setSource(Common::SharedPtr<RenderPrimitive> source)
    {
        _source = Common::move(source);
    }

    void RenderPickHighlight::setSourceShader(Common::SharedPtr<Shader> shader)
    {
        _sourceShader = Common::move(shader);
    }

    void RenderPickHighlight::setIndices(Common::DynamicArray<IdxType> &&indices)
    {
        _indices = Common::move(indices);
    }

    void RenderPickHighlight::draw(Common::SharedPtr<Shader> shader)
    {
        if (isEmpty() || !_sourceShader)
        {
            return;
        }

        // The source primitive owns the highlight uniforms, so its own shader is used here.
        // Batching keeps large selections cheap: a whole-mesh selection becomes a few draw
        // calls instead of one per element.
        (void)shader;
        _source->drawHighlightBatch(_sourceShader, _indices);
    }

    Common::BBox<Scalar, 3> RenderPickHighlight::getBounds() const
    {
        if (!_source)
        {
            return Common::BBox<Scalar, 3>();
        }
        return _source->getBounds();
    }
} // namespace EgLab::RE
