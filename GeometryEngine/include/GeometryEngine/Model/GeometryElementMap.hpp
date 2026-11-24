#pragma once

#include <cstddef>

#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "GeometryEngine/Common/GeometryAPI.hpp"
#include "GeometryEngine/Common/GeometryTypes.hpp"
#include "GeometryEngine/Topology/TopologyType.hpp"

namespace EgLab::GE
{
    class GeometryDocument;

    /// Identifies one topology feature by type + index inside the owning document.
    struct GeometryEngineAPI FeatureRef
    {
        IdxType featureType{INVALID_IDX};  // TopologyType
        IdxType featureIndex{INVALID_IDX}; // index inside the array of that type

        FeatureRef() = default;

        FeatureRef(TopologyType type, IdxType index)
            : featureType(static_cast<IdxType>(type)), featureIndex(index)
        {
        }

        bool isValid() const
        {
            return featureType != INVALID_IDX && featureIndex != INVALID_IDX;
        }

        TopologyType getType() const
        {
            return static_cast<TopologyType>(featureType);
        }

        bool operator==(const FeatureRef& other) const
        {
            return featureType == other.featureType && featureIndex == other.featureIndex;
        }

        bool operator!=(const FeatureRef& other) const
        {
            return !(*this == other);
        }
    };

    /// Element id <-> feature lookups over a tessellated model.
    ///
    /// The forward table (element -> owning feature) is supplied by whoever built the render
    /// mesh; the inverse table (feature -> elements) is derived here so feature level picking
    /// and selection can be answered in linear time. Element ids are 1-based, so id 0 keeps
    /// its "no element" meaning.
    class GeometryEngineAPI GeometryElementMap
    {
    public:
        void clear();

        /// Builds both lookup directions.
        /// @param document Document that owns the topology referenced by elementFeatures.
        ///        It must outlive the map, because aggregate features are expanded through it.
        /// @param elementFeatures elementFeatures[elemId - 1] owns element elemId.
        Common::Return build(const GeometryDocument& document,
                             const Common::DynamicArray<FeatureRef>& elementFeatures);

        std::size_t getElementCount() const
        {
            return _elementFeatures.size();
        }

        /// Returns the feature owning elemId, or an invalid reference when it is unknown.
        FeatureRef getElementFeature(IdType elemId) const;

        /// Appends every element owned by ref, ascending and without duplicates.
        /// Aggregate features (a solid) contribute the elements of the faces they contain.
        void collectElements(const FeatureRef& ref, Common::DynamicArray<IdType>& outElemIds) const;

    private:
        IdxType findSlot(const FeatureRef& ref) const;

        void collectSlotElements(IdxType slot, Common::DynamicArray<IdType>& outElemIds) const;

        void collectFeatureElements(const FeatureRef& ref, int depth,
                                    Common::DynamicArray<IdType>& outElemIds) const;

        const GeometryDocument* _document{nullptr};
        Common::DynamicArray<FeatureRef> _elementFeatures;
        Common::DynamicArray<FeatureRef> _featureRefs;
        Common::DynamicArray<IdxType> _featureElemOffsets; // slot -> begin of its element range
        Common::DynamicArray<IdType> _featureElemIds;      // flat per-feature element lists
    };

} // namespace EgLab::GE
