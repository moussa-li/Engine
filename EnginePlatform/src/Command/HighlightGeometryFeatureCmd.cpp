#include "Command/HighlightGeometryFeatureCmd.hpp"

#include "Common/HashSet.hpp"
#include "Common/Log.hpp"
#include "DataBase/Context.hpp"
#include "DataBase/GeometryData.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    HighlightGeometryFeatureCmd::HighlightGeometryFeatureCmd(
        const GeometryFeatureHighlightParam& params)
        : CommandBase<GeometryFeatureHighlightParam>(params)
    {
    }

    Common::Return HighlightGeometryFeatureCmd::execImpl()
    {
        if (getParams().count == 0)
        {
            RenderWork::instance().clearGeometryFeatureHighlight();
            return Common::Return::Succeed;
        }

        auto node = Context::instance().getDataNode(getParams().dataNodeId);
        auto* geometryData = dynamic_cast<GeometryData*>(node.get());
        if (geometryData == nullptr)
        {
            LOG(WARNING) << "HighlightGeometryFeatureCmd::exec() could not find geometry data node "
                         << getParams().dataNodeId;
            return Common::Return::Failed;
        }

        const auto mesh = geometryData->getMesh();
        if (!mesh)
        {
            LOG(WARNING) << "HighlightGeometryFeatureCmd::exec() found a geometry node without a "
                            "mesh";
            return Common::Return::Failed;
        }

        // One click can select features of several types (for example a face and one of its
        // edges), so the render ids of every feature are merged into a single highlight set.
        Common::HashSet<uint32_t> elementIds;
        Common::HashSet<uint32_t> segmentIndices;
        Common::HashSet<uint32_t> nodeIndices;
        for (uint32_t index = 0; index < getParams().count; ++index)
        {
            const auto type = static_cast<GE::TopologyType>(getParams().featureTypes[index]);
            const GeometryFeatureRef ref(type, getParams().featureIndices[index]);

            GeometryFeatureRenderIds ids;
            geometryData->collectFeatureRenderIds(ref, ids);

            for (const uint32_t elemId : ids.elementIds)
            {
                elementIds.insert(elemId);
            }
            for (const uint32_t segmentIndex : ids.edgeSegmentIndices)
            {
                segmentIndices.insert(segmentIndex);
            }
            for (const uint32_t nodeIndex : ids.nodeIndices)
            {
                nodeIndices.insert(nodeIndex);
            }
        }

        Common::DynamicArray<uint32_t> mergedElementIds;
        mergedElementIds.reserve(elementIds.size());
        for (auto it = elementIds.begin(); it.hasNext(); it.next())
        {
            mergedElementIds.pushBack(it.data());
        }

        // The render side draws edge highlights from mesh node pairs, not segment indices.
        Common::DynamicArray<uint32_t> edgeSegmentNodeIndices;
        const auto& segmentNodeIndices = geometryData->getEdgeSegmentIndices();
        edgeSegmentNodeIndices.reserve(segmentIndices.size() * 2);
        for (auto it = segmentIndices.begin(); it.hasNext(); it.next())
        {
            const std::size_t firstNode = static_cast<std::size_t>(it.data()) * 2;
            if (firstNode + 1 >= segmentNodeIndices.size())
            {
                continue;
            }
            edgeSegmentNodeIndices.pushBack(segmentNodeIndices[firstNode]);
            edgeSegmentNodeIndices.pushBack(segmentNodeIndices[firstNode + 1]);
        }

        Common::DynamicArray<uint32_t> mergedNodeIndices;
        mergedNodeIndices.reserve(nodeIndices.size());
        for (auto it = nodeIndices.begin(); it.hasNext(); it.next())
        {
            mergedNodeIndices.pushBack(it.data());
        }

        RenderWork::instance().setGeometryFeatureHighlight(mesh, mergedElementIds,
                                                           edgeSegmentNodeIndices,
                                                           mergedNodeIndices);
        LOG(INFO) << "HighlightGeometryFeatureCmd::exec() highlighted " << getParams().count
                  << " feature(s): " << mergedElementIds.size() << " element(s), "
                  << segmentIndices.size() << " segment(s), " << mergedNodeIndices.size()
                  << " node(s)";
        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform

