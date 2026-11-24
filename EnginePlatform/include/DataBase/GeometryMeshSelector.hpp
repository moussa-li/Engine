#pragma once

#include <atomic>
#include <cstdint>

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "DataBase/GeometryData.hpp"
#include "DataBase/GeometryFeatureMask.hpp"
#include "RenderEngine/Core/MeshSelector.hpp"

namespace EgLab::ME
{
    class Mesh;
}

namespace EgLab::Platform
{
    /// Mesh selector that resolves picks and selections at B-Rep feature level.
    ///
    /// A geometry model is rendered as a Tri3 mesh, so one primitive element is one triangle.
    /// This selector expands such a hit to the complete topology feature that owns the
    /// triangle (all of its triangles are highlighted) and applies the feature type filter
    /// configured by the geometry info panel.
    class GeometryMeshSelector : public RE::MeshSelector
    {
    public:
        GeometryMeshSelector() = default;

        ~GeometryMeshSelector() override = default;

        /// Restricts which topology types may be picked or selected.
        /// @param mask Combination of GEOMETRY_FEATURE_MASK_* bits.
        void setFeatureTypeMask(uint32_t mask)
        {
            _featureTypeMask.store(mask & GEOMETRY_FEATURE_MASK_ALL);
        }

        uint32_t getFeatureTypeMask() const
        {
            return _featureTypeMask.load();
        }

        /// @brief Highlight indices of the complete feature that owns the picked triangle.
        Common::DynamicArray<RE::IdxType> getHighlightIndices(
            const Common::SharedPtr<RE::RenderPrimitive>& primitive,
            RE::IdxType pickedIndex) const override;

        /// @brief Expands the resolved elements of a rectangle pick to whole features.
        bool resolveRectSelection(const Common::SharedPtr<RE::RenderPrimitive>& primitive,
                                  const Common::DynamicArray<RE::IdxType>& indices,
                                  RE::RectSelectionResolution& resolution) const override;

        /// @brief Rejects hits whose feature type is filtered out.
        bool acceptsPick(const Common::SharedPtr<RE::RenderPrimitive>& primitive,
                         RE::IdxType pickedIndex) const override;

    private:
        /// A picked element resolved to the feature that should represent it.
        struct FeatureHit
        {
            Common::SharedPtr<GeometryData> data;
            GeometryFeatureRef ref;

            bool isValid() const
            {
                return data != nullptr && ref.isValid();
            }
        };

        /// Resolves a primitive element to its feature, applying the type filter.
        FeatureHit resolveFeatureHit(const Common::SharedPtr<RE::RenderPrimitive>& primitive,
                                     RE::IdxType pickedIndex) const;

        /// Returns the element id of a primitive element, or 0 when it has no mapping.
        static uint32_t elementIdOf(const MeshRegistration& registration,
                                    const Common::SharedPtr<RE::RenderPrimitive>& primitive,
                                    RE::IdxType pickedIndex);

        /// Appends the triangles of the feature to outIndices, ascending.
        static void collectFeatureTriangles(const MeshRegistration& registration,
                                            const GE::GeometryElementMap& map,
                                            const GeometryFeatureRef& ref,
                                            Common::DynamicArray<RE::IdxType>& outIndices);

        std::atomic<uint32_t> _featureTypeMask{GEOMETRY_FEATURE_MASK_ALL};
    };

} // namespace EgLab::Platform
