#include "MeshPrimitiveCreator.hpp"

#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/RenderFace.hpp"
#include "RenderEngine/Core/RenderLine.hpp"
#include "RenderEngine/Core/RenderNode.hpp"

namespace EgLab::RE
{

    MeshPrimitiveCreator::MeshPrimitiveCreator(Common::SharedPtr<ME::Mesh> mesh) : _mesh(mesh)
    {
        _translator.setMesh(mesh);
    }

    MeshPrimitiveCreator::~MeshPrimitiveCreator()
    {
    }

    void MeshPrimitiveCreator::updateData(Common::SharedPtr<RenderNode> primitive)
    {
        ME::MeshIterator meshIt(*_mesh);
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
        ME::MeshIterator meshIt(*_mesh);
        Common::DynamicArray<RE::CoordType> nodes;

        do
        {
            auto& n = meshIt.currentNode();
            RE::CoordType c(n.x(), n.y(), n.z());
            nodes.pushBack(c);
        } while (meshIt.nextNode());

        primitive->setNodes(Common::move(nodes));

        primitive->setIndices(_translator.getBoundaryLineIdx());
    }

    void MeshPrimitiveCreator::updateData(Common::SharedPtr<RenderFace> primitive)
    {
        ME::MeshIterator meshIt(*_mesh);
        Common::DynamicArray<RE::CoordType> nodes;
        do
        {
            auto& n = meshIt.currentNode();
            RE::CoordType c(n.x(), n.y(), n.z());
            nodes.pushBack(c);
        } while (meshIt.nextNode());

        primitive->setNodes(Common::move(nodes));

        primitive->setIndices(_translator.getBoundaryFaceIdx());

        // Common::DynamicArray<RE::IdxType> idxs;

        // do
        // {
        //     auto& e = meshIt.currentElem();
        //     const auto& elemNodes = e.getNodes();
        //     for (auto node : elemNodes)
        //     {
        //     }
        // } while (meshIt.nextElem());

        // primitive->setIndices(Common::move(idxs));
    }

} // namespace EgLab::RE