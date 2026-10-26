#include "MeshPrimitiveCreator.hpp"

#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/RenderFace.hpp"
#include "RenderEngine/Core/RenderLine.hpp"
#include "RenderEngine/Core/RenderNode.hpp"

namespace EgLab::RE
{

    MeshPrimitiveCreator::MeshPrimitiveCreator(Common::SharedPtr<ME::Mesh> mesh)
        : _mesh(mesh), _translator(Common::makeShared<MeshToTriFace>())
    {
        _translator->setMesh(mesh);
    }

    MeshPrimitiveCreator::~MeshPrimitiveCreator()
    {
    }

    Common::SharedPtr<MeshToTriFace> MeshPrimitiveCreator::getTranslator() const
    {
        return _translator;
    }

    void MeshPrimitiveCreator::updateData(Common::SharedPtr<RenderNode> primitive)
    {
        ME::MeshIterator meshIt(*_mesh);
        Common::DynamicArray<RE::CoordType> nodes;
        nodes.reserve(_mesh->getNodeNumber());

        for (; meshIt.hasMoreNode(); meshIt.nextNode())
        {
            auto& n = meshIt.currentNode();
            RE::CoordType c(n.x(), n.y(), n.z());
            nodes.pushBack(c);
        }

        primitive->setNodes(Common::move(nodes));
    }

    void MeshPrimitiveCreator::updateData(Common::SharedPtr<RenderLine> primitive)
    {
        ME::MeshIterator meshIt(*_mesh);
        Common::DynamicArray<RE::CoordType> nodes;
        nodes.reserve(_mesh->getNodeNumber());
        for (; meshIt.hasMoreNode(); meshIt.nextNode())
        {
            auto& n = meshIt.currentNode();
            RE::CoordType c(n.x(), n.y(), n.z());
            nodes.pushBack(c);
        }

        primitive->setNodes(Common::move(nodes));

        primitive->setIndices(_translator->getBoundaryLineIdx());
    }

    void MeshPrimitiveCreator::updateData(Common::SharedPtr<RenderFace> primitive)
    {
        ME::MeshIterator meshIt(*_mesh);
        Common::DynamicArray<RE::CoordType> nodes;
        nodes.reserve(_mesh->getNodeNumber());
        for (; meshIt.hasMoreNode(); meshIt.nextNode())
        {
            auto& n = meshIt.currentNode();
            RE::CoordType c(n.x(), n.y(), n.z());
            nodes.pushBack(c);
        }

        Common::DynamicArray<RE::CoordType> faceNodes;
        const auto& faceNodeIndices = _translator->getBoundaryFaceNodeIdx();
        faceNodes.reserve(faceNodeIndices.size());
        for (IdxType sourceNodeIndex : faceNodeIndices)
        {
            if (sourceNodeIndex < nodes.size()) faceNodes.pushBack(nodes[sourceNodeIndex]);
        }

        primitive->setNodes(Common::move(faceNodes));
        Common::DynamicArray<IdxType> sourceIndices = _translator->getBoundaryFaceIdx();
        primitive->setIndices(Common::move(sourceIndices));

        static int testTime = 0;
        testTime++;

        Common::Vector4f color(testTime * 0.1f, 0.5f, 0.5f, 1.0f);
        IdxType triangleCount = static_cast<IdxType>(sourceIndices.size() / 3);

        Common::DynamicArray<Common::Vector4f> colors;
        colors.reserve(triangleCount);
        for (size_t i = 0; i < triangleCount; ++i)
        {
            colors.pushBack(color);
        }
        primitive->setColors(Common::move(colors));
    }

} // namespace EgLab::RE