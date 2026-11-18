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
        (void)shader;
        for (size_t index = 0; index < _indices.size(); ++index)
        {
            _source->drawHighlight(_sourceShader, _indices[index]);
        }
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
