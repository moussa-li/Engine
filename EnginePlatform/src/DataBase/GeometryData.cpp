#include "DataBase/GeometryData.hpp"

#include <mutex>

#include "DataBase/Context.hpp"
#include "DataBase/DataTree.hpp"
#include "DataBase/GeometryFeatureMask.hpp"
#include "GeometryEngine/Model/GeometryDocument.hpp"
#include "GeometryEngine/Model/GeometryFeature.hpp"

namespace EgLab::Platform
{
    namespace
    {
        Common::SharedPtr<GeometryData> findByMeshIn(
            const Common::DynamicArray<Common::SharedPtr<TreeNode>>& nodes, uint64_t meshId)
        {
            const ME::Mesh* mesh = reinterpret_cast<const ME::Mesh*>(meshId);
            for (const auto& node : nodes)
            {
                auto geometryData = Common::dynamicSharedPtrCast<GeometryData>(node);
                if (geometryData && geometryData->getMesh().get() == mesh)
                {
                    return geometryData;
                }

                auto descendant = findByMeshIn(node->getChildren(), meshId);
                if (descendant)
                {
                    return descendant;
                }
            }
            return nullptr;
        }
    } // namespace

    GeometryData::GeometryData(DataNodeId id, const Common::String& name,
                               const Common::SharedPtr<ME::Mesh>& mesh,
                               const Common::SharedPtr<GE::GeometryDocument>& document,
                               const Common::String& sourcePath)
        : MeshData(id, name, mesh), _document(document), _sourcePath(sourcePath)
    {
    }

    Common::SharedPtr<GE::GeometryDocument> GeometryData::getDocument() const
    {
        std::lock_guard<std::mutex> lock(_geometryMutex);
        return _document;
    }

    void GeometryData::setDocument(const Common::SharedPtr<GE::GeometryDocument>& document)
    {
        std::lock_guard<std::mutex> lock(_geometryMutex);
        _document = document;
    }

    const Common::String& GeometryData::getSourcePath() const
    {
        return _sourcePath;
    }

    void GeometryData::setElementFeatures(const Common::DynamicArray<GeometryFeatureRef>& refs)
    {
        std::lock_guard<std::mutex> lock(_geometryMutex);
        _elementFeatures.clear();
        for (std::size_t index = 0; index < refs.size(); ++index)
        {
            _elementFeatures.pushBack(refs[index]);
        }

        _elementMap.clear();
        if (_document)
        {
            if (_elementMap.build(*_document, _elementFeatures) != Common::Return::Succeed)
            {
                _elementMap.clear();
            }
        }
    }

    const GE::GeometryElementMap& GeometryData::getElementMap() const
    {
        return _elementMap;
    }

    void GeometryData::setVertexEdgeFeatures(
        const Common::DynamicArray<GeometryFeatureRef>& nodeFeatures,
        const Common::DynamicArray<uint32_t>& edgeSegmentIndices,
        const Common::DynamicArray<GeometryFeatureRef>& edgeSegmentFeatures)
    {
        std::lock_guard<std::mutex> lock(_geometryMutex);

        _nodeFeatures.clear();
        for (std::size_t index = 0; index < nodeFeatures.size(); ++index)
        {
            _nodeFeatures.pushBack(nodeFeatures[index]);
        }

        _edgeSegmentIndices.clear();
        for (std::size_t index = 0; index < edgeSegmentIndices.size(); ++index)
        {
            _edgeSegmentIndices.pushBack(edgeSegmentIndices[index]);
        }

        _edgeSegmentFeatures.clear();
        for (std::size_t index = 0; index < edgeSegmentFeatures.size(); ++index)
        {
            _edgeSegmentFeatures.pushBack(edgeSegmentFeatures[index]);
        }
    }

    void GeometryData::setNodeUvs(const Common::DynamicArray<Common::Vector2f>& uvs)
    {
        std::lock_guard<std::mutex> lock(_geometryMutex);
        _nodeUvs.clear();
        for (std::size_t index = 0; index < uvs.size(); ++index)
        {
            _nodeUvs.pushBack(uvs[index]);
        }
    }

    const Common::DynamicArray<Common::Vector2f>& GeometryData::getNodeUvs() const
    {
        return _nodeUvs;
    }

    GeometryFeatureRef GeometryData::getFeatureByNodeIndex(uint32_t nodeIndex) const
    {
        std::lock_guard<std::mutex> lock(_geometryMutex);
        if (nodeIndex >= _nodeFeatures.size())
        {
            return GeometryFeatureRef();
        }
        return _nodeFeatures[nodeIndex];
    }

    GeometryFeatureRef GeometryData::getFeatureByNodeId(uint32_t nodeId) const
    {
        if (nodeId == 0)
        {
            return GeometryFeatureRef();
        }
        return getFeatureByNodeIndex(nodeId - 1);
    }

    GeometryFeatureRef GeometryData::getFeatureByEdgeSegment(uint32_t segmentIndex) const
    {
        std::lock_guard<std::mutex> lock(_geometryMutex);
        if (segmentIndex >= _edgeSegmentFeatures.size())
        {
            return GeometryFeatureRef();
        }
        return _edgeSegmentFeatures[segmentIndex];
    }

    bool GeometryData::isVertexNode(uint32_t nodeIndex) const
    {
        return getFeatureByNodeIndex(nodeIndex).isValid();
    }

    const Common::DynamicArray<uint32_t>& GeometryData::getEdgeSegmentIndices() const
    {
        return _edgeSegmentIndices;
    }

    bool GeometryData::hasBrepEdges() const
    {
        return _edgeSegmentIndices.size() >= 2 && !_edgeSegmentFeatures.empty();
    }

    GeometryFeatureRef GeometryData::findParentFeature(const GeometryFeatureRef& ref,
                                                       GE::TopologyType parentType) const
    {
        if (!ref.isValid() ||
            static_cast<uint32_t>(parentType) >= GE::TOPOLOGY_TYPE_COUNT)
        {
            return GeometryFeatureRef();
        }

        std::lock_guard<std::mutex> lock(_geometryMutex);
        if (!_document)
        {
            return GeometryFeatureRef();
        }

        // Parents of a node are indices into the array of the next higher dimension.
        GE::TopologyType type = ref.getType();
        GE::IdxType index = ref.featureIndex;
        while (static_cast<uint32_t>(type) < static_cast<uint32_t>(parentType))
        {
            const GE::GeometryFeature feature = _document->getFeature(type, index);
            if (!feature.isValid() || feature.getParents().empty())
            {
                return GeometryFeatureRef();
            }
            type = static_cast<GE::TopologyType>(static_cast<uint32_t>(type) + 1);
            index = feature.getParents()[0];
        }

        if (type != parentType)
        {
            return GeometryFeatureRef();
        }
        return GeometryFeatureRef(type, index);
    }

    GeometryFeatureRef GeometryData::resolveByMask(const GeometryFeatureRef& ref,
                                                   uint32_t mask) const
    {
        if (!ref.isValid())
        {
            return GeometryFeatureRef();
        }
        if (GeometryFeatureMaskInfo::isEnabled(mask, ref.featureType))
        {
            return ref;
        }

        // A face hit stands for the solid that owns it, so filtering faces out while keeping
        // solids enabled turns every hover into a whole-solid pick.
        if (ref.getType() == GE::TopologyType::Face &&
            GeometryFeatureMaskInfo::isEnabled(mask, static_cast<uint32_t>(GE::TopologyType::Solid)))
        {
            return findParentFeature(ref, GE::TopologyType::Solid);
        }

        return GeometryFeatureRef();
    }

    GeometryFeatureRef GeometryData::getFeatureByElement(uint32_t elemId) const
    {
        if (elemId == 0)
        {
            return GeometryFeatureRef{};
        }

        std::lock_guard<std::mutex> lock(_geometryMutex);
        const std::size_t index = static_cast<std::size_t>(elemId - 1);
        if (index >= _elementFeatures.size())
        {
            return GeometryFeatureRef{};
        }
        return _elementFeatures[index];
    }

    GeometryFeatureInfo GeometryData::getFeatureInfo(const GeometryFeatureRef& ref) const
    {
        GeometryFeatureInfo info;
        if (!ref.isValid())
        {
            return info;
        }

        std::lock_guard<std::mutex> lock(_geometryMutex);
        if (!_document)
        {
            return info;
        }

        const auto type = static_cast<GE::TopologyType>(ref.featureType);
        const GE::GeometryFeature feature = _document->getFeature(type, ref.featureIndex);
        if (!feature.isValid())
        {
            return info;
        }

        info.ref = ref;
        info.featureId = feature.getId();
        info.geometryType = static_cast<uint32_t>(feature.getGeometry().getType());
        info.parentCount = static_cast<uint32_t>(feature.getParents().size());
        info.childCount = static_cast<uint32_t>(feature.getChildren().size());
        info.name = feature.getName();

        const GE::Geometry& geometry = feature.getGeometry();
        info.length = geometry.getLength();
        info.area = geometry.getArea();
        info.volume = geometry.getVolume();

        const GE::BoundingBox& box = geometry.getBoundingBox();
        if (!box.isEmpty())
        {
            const GE::Vec3 minPoint = box.min();
            const GE::Vec3 maxPoint = box.max();
            for (std::size_t axis = 0; axis < 3; ++axis)
            {
                info.bboxMin[axis] = static_cast<float>(minPoint[axis]);
                info.bboxMax[axis] = static_cast<float>(maxPoint[axis]);
            }
            info.x = 0.5 * (minPoint[0] + maxPoint[0]);
            info.y = 0.5 * (minPoint[1] + maxPoint[1]);
            info.z = 0.5 * (minPoint[2] + maxPoint[2]);
        }

        return info;
    }

    GeometryFeatureInfo GeometryData::getFeatureInfo(uint32_t elemId) const
    {
        const GeometryFeatureRef ref = getFeatureByElement(elemId);
        GeometryFeatureInfo info = getFeatureInfo(ref);
        if (info.ref.isValid())
        {
            info.elemId = elemId;
        }
        return info;
    }

    Common::SharedPtr<GeometryData> GeometryData::findByMesh(uint64_t meshId)
    {
        if (meshId == 0)
        {
            return nullptr;
        }
        return findByMeshIn(Context::instance().getDataTree().getChildren(), meshId);
    }

} // namespace EgLab::Platform
