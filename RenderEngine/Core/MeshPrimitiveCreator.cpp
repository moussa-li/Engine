#include "MeshPrimitiveCreator.hpp"

#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/RenderFace.hpp"
#include "RenderEngine/Core/RenderLine.hpp"
#include "RenderEngine/Core/RenderNode.hpp"


namespace EgLab::RE
{

    MeshPrimitiveCreator::MeshPrimitiveCreator(ME::Mesh& mesh) : _mesh(mesh)
    {
    }

    void MeshPrimitiveCreator::updateData(Common::SharedPtr<RenderNode> primitive)
    {
        ME::MeshIterator meshIt(_mesh);
        Common::DynamicArray<RE::CoordType> nodes;

        do
        {
            auto& n = meshIt.currentNode();
            RE::CoordType c(n.x(), n.y(), n.z());
            nodes.pushBack(c);
        } while (meshIt.nextNode());

        primitive->setNodes(Common::move(nodes));
    }

    void MeshPrimitiveCreator::updateData(Common::SharedPtr<RenderLine> primitive)
    {
    }

    void MeshPrimitiveCreator::updateData(Common::SharedPtr<RenderFace> primitive)
    {
    }

} // namespace EgLab::RE