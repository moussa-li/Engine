#include "GeometryEngine/Model/GeometryElementMap.hpp"

#include "GeometryEngine/Model/GeometryDocument.hpp"
#include "GeometryEngine/Model/GeometryFeature.hpp"

namespace EgLab::GE
{
    namespace
    {
        constexpr int MAX_CHILD_DEPTH = static_cast<int>(TOPOLOGY_TYPE_COUNT);
    } // namespace

    void GeometryElementMap::clear()
    {
        _document = nullptr;
        _elementFeatures.clear();
        _featureRefs.clear();
        _featureElemOffsets.clear();
        _featureElemIds.clear();
        for (std::size_t type = 0; type < TYPE_COUNT; ++type)
        {
            _slotOfFeature[type].clear();
        }
    }

    Common::Return GeometryElementMap::build(const GeometryDocument& document,
                                             const Common::DynamicArray<FeatureRef>& elementFeatures)
    {
        clear();
        if (elementFeatures.empty())
        {
            return Common::Return::Succeed;
        }

        _document = &document;
        _elementFeatures.reserve(elementFeatures.size());
        for (std::size_t index = 0; index < elementFeatures.size(); ++index)
        {
            _elementFeatures.pushBack(elementFeatures[index]);
        }

        Common::DynamicArray<IdxType> slotOfElement;
        Common::DynamicArray<IdxType> counts;
        slotOfElement.reserve(_elementFeatures.size());
        for (std::size_t index = 0; index < _elementFeatures.size(); ++index)
        {
            const FeatureRef& ref = _elementFeatures[index];
            IdxType slot = findSlot(ref);
            if (slot == INVALID_IDX)
            {
                slot = static_cast<IdxType>(_featureRefs.size());
                _featureRefs.pushBack(ref);
                counts.pushBack(0);
                registerSlot(ref, slot);
            }

            slotOfElement.pushBack(slot);
            counts[slot] = counts[slot] + 1;
        }

        _featureElemOffsets.resize(counts.size() + 1);
        _featureElemOffsets[0] = 0;
        for (std::size_t slot = 0; slot < counts.size(); ++slot)
        {
            _featureElemOffsets[slot + 1] = _featureElemOffsets[slot] + counts[slot];
        }

        _featureElemIds.resize(_elementFeatures.size());
        for (std::size_t slot = 0; slot < counts.size(); ++slot)
        {
            counts[slot] = _featureElemOffsets[slot];
        }
        for (std::size_t index = 0; index < slotOfElement.size(); ++index)
        {
            const IdxType slot = slotOfElement[index];
            _featureElemIds[counts[slot]] = static_cast<IdType>(index + 1);
            counts[slot] = counts[slot] + 1;
        }

        return Common::Return::Succeed;
    }

    FeatureRef GeometryElementMap::getElementFeature(IdType elemId) const
    {
        if (elemId == 0)
        {
            return FeatureRef();
        }
        const std::size_t index = static_cast<std::size_t>(elemId - 1);
        if (index >= _elementFeatures.size())
        {
            return FeatureRef();
        }
        return _elementFeatures[index];
    }

    void GeometryElementMap::collectElements(const FeatureRef& ref,
                                             Common::DynamicArray<IdType>& outElemIds) const
    {
        if (!ref.isValid())
        {
            return;
        }
        collectFeatureElements(ref, 0, outElemIds);
    }

    void GeometryElementMap::collectFeatureElements(const FeatureRef& ref, int depth,
                                                    Common::DynamicArray<IdType>& outElemIds) const
    {
        const IdxType slot = findSlot(ref);
        if (slot != INVALID_IDX)
        {
            collectSlotElements(slot, outElemIds);
        }

        if (_document == nullptr || depth >= MAX_CHILD_DEPTH)
        {
            return;
        }

        TopologyType childType = TopologyType::Vertex;
        if (!TopologyTypeInfo::tryGetChildType(ref.getType(), childType))
        {
            return;
        }

        const GeometryFeature feature = _document->getFeature(ref.getType(), ref.featureIndex);
        if (!feature.isValid())
        {
            return;
        }

        const Common::DynamicArray<IdxType>& children = feature.getChildren();
        for (std::size_t index = 0; index < children.size(); ++index)
        {
            collectFeatureElements(FeatureRef(childType, children[index]), depth + 1, outElemIds);
        }
    }

    void GeometryElementMap::collectSlotElements(IdxType slot,
                                                 Common::DynamicArray<IdType>& outElemIds) const
    {
        const IdxType begin = _featureElemOffsets[slot];
        const IdxType end = _featureElemOffsets[slot + 1];
        for (IdxType index = begin; index < end; ++index)
        {
            outElemIds.pushBack(_featureElemIds[index]);
        }
    }

    IdxType GeometryElementMap::findSlot(const FeatureRef& ref) const
    {
        if (ref.featureType >= TYPE_COUNT)
        {
            return INVALID_IDX;
        }
        const Common::DynamicArray<IdxType>& table = _slotOfFeature[ref.featureType];
        if (ref.featureIndex >= table.size())
        {
            return INVALID_IDX;
        }
        return table[ref.featureIndex];
    }

    void GeometryElementMap::registerSlot(const FeatureRef& ref, IdxType slot)
    {
        if (ref.featureType >= TYPE_COUNT)
        {
            return;
        }

        Common::DynamicArray<IdxType>& table = _slotOfFeature[ref.featureType];
        if (ref.featureIndex >= table.size())
        {
            const std::size_t oldSize = table.size();
            table.resize(static_cast<std::size_t>(ref.featureIndex) + 1);
            for (std::size_t index = oldSize; index < table.size(); ++index)
            {
                table[index] = INVALID_IDX;
            }
        }
        table[ref.featureIndex] = slot;
    }

} // namespace EgLab::GE
