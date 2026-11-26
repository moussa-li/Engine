#include "RenderEngine/Core/MeshPrimitiveCreator.hpp"

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

    MeshPrimitiveCreator::MeshPrimitiveCreator(Common::SharedPtr<ME::Mesh> mesh,
                                               Common::SharedPtr<MeshToTriFace> translator)
        : _mesh(mesh), _translator(translator)
    {
    }

    MeshPrimitiveCreator::~MeshPrimitiveCreator()
    {
    }

    Common::SharedPtr<MeshToTriFace> MeshPrimitiveCreator::getTranslator() const
    {
        return _translator;
    }

    void MeshPrimitiveCreator::setLineIndexSource(LineIndexSource source)
    {
        _lineIndexSource = source;
    }

    void MeshPrimitiveCreator::setSmoothNormals(bool smooth)
    {
        _smoothNormals = smooth;
    }

    void MeshPrimitiveCreator::setNodeUvs(const Common::DynamicArray<Common::Vector2f>* uvs)
    {
        _nodeUvs = uvs;
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

        // BoundaryOnly keeps only the tessellation patch outline, which for B-Rep geometry is
        // the face frame; AllTriangleEdges keeps the dense triangle wireframe.
        primitive->setIndices(_lineIndexSource == LineIndexSource::BoundaryOnly
                                  ? _translator->getBoundaryElementEdgeIdx()
                                  : _translator->getBoundaryLineIdx());
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

        // Surface UVs are expanded per face corner so they line up with faceNodes (and with the
        // de-indexed vertex array the face primitive builds), letting the UV shader interpolate
        // them across each triangle.
        if (_nodeUvs != nullptr && !_nodeUvs->empty())
        {
            Common::DynamicArray<Common::Vector2f> faceUvs;
            faceUvs.reserve(faceNodeIndices.size());
            for (IdxType sourceNodeIndex : faceNodeIndices)
            {
                if (sourceNodeIndex < _nodeUvs->size())
                {
                    faceUvs.pushBack((*_nodeUvs)[sourceNodeIndex]);
                }
                else
                {
                    faceUvs.pushBack(Common::Vector2f(0.0f, 0.0f));
                }
            }
            primitive->setUvs(Common::move(faceUvs));
        }

        if (_smoothNormals)
        {
            // Average the triangle normals per mesh node. Geometry models duplicate nodes
            // between different B-Rep faces, so one face shades smoothly while different
            // faces keep a sharp boundary.
            Common::DynamicArray<RE::CoordType> accumulated;
            accumulated.resize(_mesh->getNodeNumber());

            ME::MeshIterator elemIt(*_mesh);
            for (; elemIt.hasMoreElem(); elemIt.nextElem())
            {
                const ME::Elem& elem = elemIt.currentElem();
                if (elem.getType() != ME::ElemType::Tri3)
                {
                    continue;
                }
                const auto& ids = elem.getNodes();
                if (ids.size() < 3)
                {
                    continue;
                }

                const ME::IdxType i0 = _mesh->getNodeIdx(ids[0]);
                const ME::IdxType i1 = _mesh->getNodeIdx(ids[1]);
                const ME::IdxType i2 = _mesh->getNodeIdx(ids[2]);
                const ME::IdxType maxIndex =
                    i0 > i1 ? (i0 > i2 ? i0 : i2) : (i1 > i2 ? i1 : i2);
                if (maxIndex >= accumulated.size())
                {
                    accumulated.resize(static_cast<size_t>(maxIndex) + 1);
                }

                const ME::Node& n0 = _mesh->getNodeById(ids[0]);
                const ME::Node& n1 = _mesh->getNodeById(ids[1]);
                const ME::Node& n2 = _mesh->getNodeById(ids[2]);
                const RE::CoordType p0(n0.x(), n0.y(), n0.z());
                const RE::CoordType p1(n1.x(), n1.y(), n1.z());
                const RE::CoordType p2(n2.x(), n2.y(), n2.z());
                const RE::CoordType normal = (p1 - p0).cross(p2 - p0);

                accumulated[i0] = accumulated[i0] + normal;
                accumulated[i1] = accumulated[i1] + normal;
                accumulated[i2] = accumulated[i2] + normal;
            }

            for (size_t i = 0; i < accumulated.size(); ++i)
            {
                accumulated[i].normalize();
            }

            Common::DynamicArray<RE::CoordType> smoothNormals;
            const auto& faceNodeIndices = _translator->getBoundaryFaceNodeIdx();
            smoothNormals.reserve(faceNodeIndices.size());
            for (IdxType sourceNodeIndex : faceNodeIndices)
            {
                if (sourceNodeIndex < accumulated.size())
                {
                    smoothNormals.pushBack(accumulated[sourceNodeIndex]);
                }
                else
                {
                    smoothNormals.pushBack(RE::CoordType(0.0f, 0.0f, 1.0f));
                }
            }
            primitive->setNormals(Common::move(smoothNormals));
        }

        // Hand the face primitive the geometric center of each boundary element face so that
        // shrinking collapses every element face toward its own center. This works for both
        // surface meshes (Tri/Quad) and volumetric meshes (Tet/Hex boundary faces).
        const auto& faceLocations = _translator->getBoundaryFaceLocations();
        Common::DynamicArray<RE::CoordType> faceCenters;
        faceCenters.reserve(faceLocations.size() * 3);
        for (size_t triangle = 0; triangle < faceLocations.size(); ++triangle)
        {
            const auto& location = faceLocations[triangle];
            ME::ElemFaceGetter faceGetter(_mesh->getElemById(location.elemId));
            const auto faceNodeIds = faceGetter.getNodes(location.faceIdx);

            RE::CoordType center(0.0f, 0.0f, 0.0f);
            for (auto nodeId : faceNodeIds)
            {
                const auto& node = _mesh->getNodeById(nodeId);
                center = center + RE::CoordType(node.x(), node.y(), node.z());
            }
            if (faceNodeIds.size() > 0)
            {
                center = center * (1.0f / static_cast<float>(faceNodeIds.size()));
            }

            faceCenters.pushBack(center);
            faceCenters.pushBack(center);
            faceCenters.pushBack(center);
        }
        primitive->setFaceCenters(Common::move(faceCenters));

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