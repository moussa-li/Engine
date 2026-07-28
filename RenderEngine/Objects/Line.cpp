#include "RenderEngine/Objects/Line.hpp"

#include "RenderEngine/Core/RenderLine.hpp"
#include "RenderEngine/Core/Shader.hpp"
#include "RenderEngine/Core/ShaderLib.hpp"
namespace EgLab
{
    Line::Line(DynamicArray<CoordType>& coords)
    {
        _shader = makeShared<Shader>(ShaderId::Node);
        _shader->bind();
        _line = makeUnique<RenderLine>();
        _line->setNodes(move(coords));
        DynamicArray<IdxType> idx;
        for (int i = 0; i < coords.size() - 1; i++)
        {
            idx.pushBack(i);
            idx.pushBack(i + 1);
        }
        _line->setIndices(move(idx));
        _line->setup();
    }

    Line::~Line()
    {
    }

    void Line::draw() const
    {
        _line->draw(_shader);
    }
} // namespace EgLab
