#include "RenderEngine/Objects/Vertex.hpp"

#include "RenderEngine/Core/RenderNode.hpp"
#include "RenderEngine/Core/Shader.hpp"
#include "RenderEngine/Core/ShaderLib.hpp"

namespace EgLab
{
    Vertex::Vertex(CoordType& coord) : Entity()
    {
        _shader = makeShared<Shader>(ShaderId::Node);
        _shader->bind();
        // _shader->unBind();

        _node = makeUnique<RenderNode>();
        DynamicArray<CoordType> coords;
        coords.pushBack(coord);

        _node->setNodes(move(coords));
        _shader->unBind();
        // LOG(INFO) << _node.get();
        _node->setup();
    }

    Vertex::~Vertex()
    {
    }

    void Vertex::draw() const
    {
        _node->draw(_shader);
    }

} // namespace EgLab
