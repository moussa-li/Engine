#include "GeometryEngine/Model/GeometryFeature.hpp"

namespace EgLab::GE
{
    namespace
    {
        const Common::DynamicArray<IdxType>& emptyIndices()
        {
            static const Common::DynamicArray<IdxType> empty;
            return empty;
        }

        const Common::String& emptyName()
        {
            static const Common::String empty;
            return empty;
        }

        const Geometry& emptyGeometry()
        {
            static const Geometry empty;
            return empty;
        }

        const Tessellation& emptyMesh()
        {
            static const Tessellation empty;
            return empty;
        }
    } // namespace

    GeometryFeature::GeometryFeature() = default;

    GeometryFeature::GeometryFeature(const TopologyNode& node, const Geometry& geometry)
        : _node(&node), _geometry(&geometry)
    {
    }

    bool GeometryFeature::isValid() const
    {
        return _node != nullptr && _geometry != nullptr;
    }

    IdType GeometryFeature::getId() const
    {
        return _node != nullptr ? _node->getId() : INVALID_ID;
    }

    const Common::String& GeometryFeature::getName() const
    {
        return _node != nullptr ? _node->getName() : emptyName();
    }

    TopologyType GeometryFeature::getType() const
    {
        return _node != nullptr ? _node->getType() : TopologyType::Vertex;
    }

    const Common::DynamicArray<IdxType>& GeometryFeature::getChildren() const
    {
        return _node != nullptr ? _node->getChildren() : emptyIndices();
    }

    const Common::DynamicArray<IdxType>& GeometryFeature::getParents() const
    {
        return _node != nullptr ? _node->getParents() : emptyIndices();
    }

    const Geometry& GeometryFeature::getGeometry() const
    {
        return _geometry != nullptr ? *_geometry : emptyGeometry();
    }

    const Tessellation& GeometryFeature::getMesh() const
    {
        return _geometry != nullptr ? _geometry->getTessellation() : emptyMesh();
    }

} // namespace EgLab::GE
