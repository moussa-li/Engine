#include "DataBase/GeometryMeshSelector.hpp"

#include "Common/HashSet.hpp"
#include "RenderEngine/Core/MeshToTriFace.hpp"

namespace EgLab::Platform
{
    Common::DynamicArray<RE::IdxType> GeometryMeshSelector::getHighlightIndices(
        const Common::SharedPtr<RE::RenderPrimitive>& primitive, RE::IdxType pickedIndex) const
    {
        const MeshRegistration* registration = findRegistration(primitive);
        if (registration == nullptr || !registration->translator ||
            primitive != registration->facePrimitive)
        {
            return RE::MeshSelector::getHighlightIndices(primitive, pickedIndex);
        }

        const FeatureHit hit = resolveFeatureHit(primitive, pickedIndex);
        if (!hit.isValid())
        {
            return RE::MeshSelector::getHighlightIndices(primitive, pickedIndex);
        }

        Common::DynamicArray<RE::IdxType> indices;
        collectFeatureTriangles(*registration, hit.data->getElementMap(), hit.ref, indices);
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

        const uint32_t elemId = elementIdOf(*registration, primitive, pickedIndex);
        if (elemId == 0)
        {
            return hit;
        }

        hit.data = GeometryData::findByMesh(reinterpret_cast<uint64_t>(registration->mesh.get()));
        if (!hit.data)
        {
            return hit;
        }

        hit.ref = hit.data->resolveByMask(hit.data->getFeatureByElement(elemId),
                                          getFeatureTypeMask());
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

} // namespace EgLab::Platform
