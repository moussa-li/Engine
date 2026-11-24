#include "GeometryEngine/Model/GeometryDocument.hpp"

namespace EgLab::GE
{
    namespace
    {
        GeometryType toGeometryType(TopologyType type)
        {
            switch (type)
            {
            case TopologyType::Vertex:
                return GeometryType::Point;
            case TopologyType::Edge:
                return GeometryType::Curve;
            case TopologyType::Face:
                return GeometryType::Surface;
            case TopologyType::Solid:
                return GeometryType::Solid;
            }
            return GeometryType::Unknown;
        }
    } // namespace

    class GeometryDocument::Impl
    {
    public:
        Common::DynamicArray<Geometry>& geometries(TopologyType type)
        {
            switch (type)
            {
            case TopologyType::Vertex:
                return vertices;
            case TopologyType::Edge:
                return edges;
            case TopologyType::Face:
                return faces;
            case TopologyType::Solid:
                return solids;
            }
            return vertices;
        }

        const Common::DynamicArray<Geometry>& geometries(TopologyType type) const
        {
            switch (type)
            {
            case TopologyType::Vertex:
                return vertices;
            case TopologyType::Edge:
                return edges;
            case TopologyType::Face:
                return faces;
            case TopologyType::Solid:
                return solids;
            }
            return vertices;
        }

        void clear()
        {
            topology.clear();
            vertices.clear();
            edges.clear();
            faces.clear();
            solids.clear();
            lastReport.reset();
        }

        Topology topology;
        Common::DynamicArray<Geometry> vertices;
        Common::DynamicArray<Geometry> edges;
        Common::DynamicArray<Geometry> faces;
        Common::DynamicArray<Geometry> solids;
        GeometryImportReport lastReport;
    };

    GeometryDocument::GeometryDocument() : _impl(Common::makeUnique<Impl>())
    {
    }

    GeometryDocument::~GeometryDocument() = default;

    Topology& GeometryDocument::getTopology()
    {
        return _impl->topology;
    }

    const Topology& GeometryDocument::getTopology() const
    {
        return _impl->topology;
    }

    IdxType GeometryDocument::createFeature(TopologyType type, const Common::String& name)
    {
        const IdxType index = _impl->topology.createNode(type, name);
        if (index == INVALID_IDX) return INVALID_IDX;

        _impl->geometries(type).pushBack(Geometry(toGeometryType(type)));
        return index;
    }

    Common::Return GeometryDocument::linkFeatures(TopologyType parentType, IdxType parentIndex,
                                                 TopologyType childType, IdxType childIndex)
    {
        return _impl->topology.link(parentType, parentIndex, childType, childIndex);
    }

    std::size_t GeometryDocument::getFeatureCount(TopologyType type) const
    {
        return _impl->topology.getNodeCount(type);
    }

    GeometryFeature GeometryDocument::getFeature(TopologyType type, IdxType index) const
    {
        const TopologyNode* node = _impl->topology.getNode(type, index);
        const Geometry* geometry = getGeometry(type, index);
        if (node == nullptr || geometry == nullptr) return GeometryFeature();
        return GeometryFeature(*node, *geometry);
    }

    Geometry* GeometryDocument::getGeometry(TopologyType type, IdxType index)
    {
        auto& storage = _impl->geometries(type);
        return index < storage.size() ? &storage[index] : nullptr;
    }

    const Geometry* GeometryDocument::getGeometry(TopologyType type, IdxType index) const
    {
        const auto& storage = _impl->geometries(type);
        return index < storage.size() ? &storage[index] : nullptr;
    }

    const GeometryImportReport& GeometryDocument::getLastReport() const
    {
        return _impl->lastReport;
    }

    void GeometryDocument::setLastReport(const GeometryImportReport& report)
    {
        _impl->lastReport = report;
    }

    void GeometryDocument::clear()
    {
        _impl->clear();
    }

} // namespace EgLab::GE
