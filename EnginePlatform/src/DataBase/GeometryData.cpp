#include "DataBase/GeometryData.hpp"

#include <mutex>

#include "DataBase/Context.hpp"
#include "DataBase/DataTree.hpp"
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
