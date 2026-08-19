#include "RenderEngine/Objects/Vertex.hpp"

#include "RenderEngine/Core/RenderNode.hpp"
#include "RenderEngine/Core/Shader.hpp"
#include "RenderEngine/Core/ShaderLib.hpp"

namespace EgLab::RE
{
    Vertex::Vertex(CoordType& coord) : Entity()
    {
        // _shader = makeShared<Shader>(ShaderId::Node);
        // _shader->bind();

        // _node = makeUnique<RenderNode>();
        // DynamicArray<CoordType> coords;
        // coords.pushBack(coord);

        // _node->setNodes(move(coords));
        // _shader->unBind();
        // _node->setup();
    }

    Vertex::~Vertex()
    {
    }

} // namespace EgLab::RE
