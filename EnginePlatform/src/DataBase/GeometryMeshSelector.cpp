#include "DataBase/GeometryMeshSelector.hpp"

#include "Common/HashSet.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "RenderEngine/Core/MeshToTriFace.hpp"

namespace EgLab::Platform
{
    namespace
    {
        constexpr uint32_t INVALID_INDEX = 0xFFFFFFFFu;
    }

    void GeometryMeshSelector::processPick(const Common::SharedPtr<RE::RenderPrimitive>& primitive,
                                           RE::IdxType pickedIndex)
    {
        const MeshRegistration* registration = findRegistration(primitive);
        if (registration == nullptr || !registration->mesh)
        {
            RE::MeshSelector::processPick(primitive, pickedIndex);
            return;
        }

        auto data = GeometryData::findByMesh(reinterpret_cast<uint64_t>(registration->mesh.get()));
        if (!data)
        {
            RE::MeshSelector::processPick(primitive, pickedIndex);
            return;
        }

        RE::PrimitivePickInfo info;
        if (primitive == registration->nodePrimitive)
        {
            // Node order equals mesh node order, so the render index maps straight to the id.
            const ME::IdType nodeId = static_cast<ME::IdType>(pickedIndex) + 1;
            if (data->getFeatureByNodeId(nodeId).isValid())
            {
                const ME::Node& node = registration->mesh->getNodeById(nodeId);
                info.kind = RE::PrimitivePickKind::Node;
                info.entityId = nodeId;
                info.position = RE::CoordType(node.x(), node.y(), node.z());
            }
        }
        else if (primitive == registration->linePrimitive && isBrepEdgeLines())
        {
            // With B-Rep edge lines the primitive index is a segment index, so the translator
            // locations do not describe the hit; faceIdx stays invalid on purpose to tell the
            // consumers that entityId is a segment and not a mesh element.
            if (data->getFeatureByEdgeSegment(pickedIndex).isValid())
            {
                info.kind = RE::PrimitivePickKind::Edge;
                info.entityId = pickedIndex;
            }
        }
        else
        {
            RE::MeshSelector::processPick(primitive, pickedIndex);
            return;
        }

        if (info.kind == RE::PrimitivePickKind::None)
        {
            publishPick(info, nullptr);
            return;
        }
        publishPick(info, registration->mesh.get());
    }

    Common::DynamicArray<RE::IdxType> GeometryMeshSelector::getHighlightIndices(
        const Common::SharedPtr<RE::RenderPrimitive>& primitive, RE::IdxType pickedIndex) const
    {
        const MeshRegistration* registration = findRegistration(primitive);
        if (registration == nullptr || !registration->translator)
        {
            return RE::MeshSelector::getHighlightIndices(primitive, pickedIndex);
        }

        const FeatureHit hit = resolveFeatureHit(primitive, pickedIndex);
        if (!hit.isValid())
        {
            return RE::MeshSelector::getHighlightIndices(primitive, pickedIndex);
        }

        Common::DynamicArray<RE::IdxType> indices;
        if (hit.ref.getType() == GE::TopologyType::Edge)
        {
            collectFeatureSegments(*hit.data, hit.ref, indices);
        }
        else if (hit.ref.getType() == GE::TopologyType::Vertex)
        {
            // A vertex feature is visualised by exactly one node.
            const RE::IdxType nodeIndex = hit.nodeId == INVALID_INDEX
                                              ? pickedIndex
                                              : static_cast<RE::IdxType>(hit.nodeId - 1);
            indices.pushBack(nodeIndex);
        }
        else
        {
            collectFeatureTriangles(*registration, hit.data->getElementMap(), hit.ref, indices);
        }

        if (indices.empty())
        {
            return RE::MeshSelector::getHighlightIndices(primitive, pickedIndex);
        }
        return indices;
    }

    bool GeometryMeshSelector::resolveRectSelection(
        const Common::SharedPtr<RE::RenderPrimitive>& primitive,
        const Common::DynamicArray<RE::IdxType>& indices,
        RE::RectSelectionResolution& resolution) const
    {
        if (!RE::MeshSelector::resolveRectSelection(primitive, indices, resolution))
        {
            return false;
        }
        if (!resolution.mesh)
        {
            return true;
        }

        Common::SharedPtr<GeometryData> data =
            GeometryData::findByMesh(reinterpret_cast<uint64_t>(resolution.mesh.get()));
        if (!data)
        {
            return true;
        }

        const MeshRegistration* registration = findRegistration(primitive);
        if (registration != nullptr && primitive == registration->linePrimitive &&
            isBrepEdgeLines())
        {
            // B-Rep edge segments are not mesh elements; face hits, which stay pickable in
            // every display mode, already resolve the rectangle to features.
            resolution.elementIds.clear();
            return true;
        }

        // A rectangle covers triangles; a geometry selection must cover whole features, so
        // every hit element is replaced by all elements of the feature that owns it.
        const uint32_t mask = getFeatureTypeMask();
        Common::HashSet<ME::IdType> expandedElements;
        for (std::size_t index = 0; index < resolution.elementIds.size(); ++index)
        {
            const GeometryFeatureRef feature =
                data->getFeatureByElement(resolution.elementIds[index]);
            const GeometryFeatureRef resolved = data->resolveByMask(feature, mask);
            if (!resolved.isValid())
            {
                // The owning feature type is filtered out.
                continue;
            }

            Common::DynamicArray<uint32_t> featureElements;
            data->getElementMap().collectElements(resolved, featureElements);
            if (featureElements.empty())
            {
                expandedElements.insert(resolution.elementIds[index]);
                continue;
            }
            for (std::size_t element = 0; element < featureElements.size(); ++element)
            {
                expandedElements.insert(featureElements[element]);
            }
        }

        resolution.elementIds.clear();
        resolution.elementIds.reserve(expandedElements.size());
        for (auto it = expandedElements.begin(); it.hasNext(); it.next())
        {
            resolution.elementIds.pushBack(it.data());
        }
        return true;
    }

    bool GeometryMeshSelector::acceptsPick(
        const Common::SharedPtr<RE::RenderPrimitive>& primitive, RE::IdxType pickedIndex) const
    {
        const MeshRegistration* registration = findRegistration(primitive);
        if (registration == nullptr || !registration->mesh)
        {
            return true;
        }
        if (!GeometryData::findByMesh(reinterpret_cast<uint64_t>(registration->mesh.get())))
        {
            // Regular meshes are never filtered by the geometry feature mask.
            return true;
        }
        return resolveFeatureHit(primitive, pickedIndex).isValid();
    }

    GeometryMeshSelector::FeatureHit GeometryMeshSelector::resolveFeatureHit(
        const Common::SharedPtr<RE::RenderPrimitive>& primitive, RE::IdxType pickedIndex) const
    {
        FeatureHit hit;
        const MeshRegistration* registration = findRegistration(primitive);
        if (registration == nullptr || !registration->mesh)
        {
            return hit;
        }

        hit.data = GeometryData::findByMesh(reinterpret_cast<uint64_t>(registration->mesh.get()));
        if (!hit.data)
        {
            return hit;
        }

        const uint32_t mask = getFeatureTypeMask();
        if (primitive == registration->nodePrimitive)
        {
            const ME::IdType nodeId = static_cast<ME::IdType>(pickedIndex) + 1;
            hit.nodeId = nodeId;
            hit.ref = hit.data->resolveByMask(hit.data->getFeatureByNodeId(nodeId), mask);
        }
        else if (primitive == registration->linePrimitive && isBrepEdgeLines())
        {
            hit.ref = hit.data->resolveByMask(hit.data->getFeatureByEdgeSegment(pickedIndex), mask);
        }
        else
        {
            const uint32_t elemId = elementIdOf(*registration, primitive, pickedIndex);
            if (elemId == 0)
            {
                hit.data = nullptr;
                return hit;
            }
            hit.ref = hit.data->resolveByMask(hit.data->getFeatureByElement(elemId), mask);
        }

        if (!hit.ref.isValid())
        {
            hit.data = nullptr;
        }
        return hit;
    }

    uint32_t GeometryMeshSelector::elementIdOf(const MeshRegistration& registration,
                                               const Common::SharedPtr<RE::RenderPrimitive>& primitive,
                                               RE::IdxType pickedIndex)
    {
        if (!registration.translator || pickedIndex == static_cast<RE::IdxType>(-1))
        {
            return 0;
        }
        if (primitive == registration.facePrimitive)
        {
            const auto& locations = registration.translator->getBoundaryFaceLocations();
            return pickedIndex < locations.size() ? locations[pickedIndex].elemId : 0;
        }
        if (primitive == registration.linePrimitive)
        {
            const auto& locations = registration.translator->getBoundaryLineLocations();
            return pickedIndex < locations.size() ? locations[pickedIndex].elemId : 0;
        }
        return 0;
    }

    void GeometryMeshSelector::collectFeatureTriangles(const MeshRegistration& registration,
                                                       const GE::GeometryElementMap& map,
                                                       const GeometryFeatureRef& ref,
                                                       Common::DynamicArray<RE::IdxType>& outIndices)
    {
        if (!registration.translator)
        {
            return;
        }

        Common::DynamicArray<uint32_t> elemIds;
        map.collectElements(ref, elemIds);
        if (elemIds.empty())
        {
            return;
        }

        const auto& locations = registration.translator->getBoundaryFaceLocations();

        // The importer appends the triangles of one feature consecutively, so the triangle
        // index is the element index shifted by one. Verified before it is relied on.
        bool directMapping = true;
        for (std::size_t index = 0; index < elemIds.size() && directMapping; ++index)
        {
            const std::size_t triangle = static_cast<std::size_t>(elemIds[index] - 1);
            directMapping = triangle < locations.size() &&
                            locations[triangle].elemId == static_cast<ME::IdType>(elemIds[index]);
        }
        if (directMapping)
        {
            outIndices.reserve(elemIds.size());
            for (std::size_t index = 0; index < elemIds.size(); ++index)
            {
                outIndices.pushBack(static_cast<RE::IdxType>(elemIds[index] - 1));
            }
            return;
        }

        Common::HashSet<ME::IdType> wantedElements;
        for (std::size_t index = 0; index < elemIds.size(); ++index)
        {
            wantedElements.insert(static_cast<ME::IdType>(elemIds[index]));
        }
        for (std::size_t index = 0; index < locations.size(); ++index)
        {
            if (wantedElements.find(locations[index].elemId) != wantedElements.end())
            {
                outIndices.pushBack(static_cast<RE::IdxType>(index));
            }
        }
    }

    void GeometryMeshSelector::collectFeatureSegments(const GeometryData& data,
                                                      const GeometryFeatureRef& ref,
                                                      Common::DynamicArray<RE::IdxType>& outIndices)
    {
        // Edge segments are laid out per feature, so the feature's segments form one range;
        // scanning finds it without keeping a second index around.
        const std::size_t segmentCount = data.getEdgeSegmentIndices().size() / 2;
        const auto matches = [&data, &ref](std::size_t segment) {
            return data.getFeatureByEdgeSegment(static_cast<uint32_t>(segment)) == ref;
        };

        std::size_t first = segmentCount;
        for (std::size_t segment = 0; segment < segmentCount; ++segment)
        {
            if (matches(segment))
            {
                first = segment;
                break;
            }
        }
        if (first >= segmentCount)
        {
            return;
        }

        outIndices.reserve(segmentCount - first);
        for (std::size_t segment = first; segment < segmentCount; ++segment)
        {
            if (!matches(segment))
            {
                break;
            }
            outIndices.pushBack(static_cast<RE::IdxType>(segment));
        }
    }

} // namespace EgLab::Platform
