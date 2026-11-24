#include "GeometryEngine/Geometry/Geometry.hpp"

namespace EgLab::GE
{
    Geometry::Geometry() = default;

    Geometry::Geometry(GeometryType type) : _type(type)
    {
    }

    GeometryType Geometry::getType() const
    {
        return _type;
    }

    void Geometry::setType(GeometryType type)
    {
        _type = type;
    }

    bool Geometry::isValid() const
    {
        return _type != GeometryType::Unknown;
    }

    double Geometry::getLength() const
    {
        return _length;
    }

    void Geometry::setLength(double length)
    {
        _length = length;
    }

    double Geometry::getArea() const
    {
        return _area;
    }

    void Geometry::setArea(double area)
    {
        _area = area;
    }

    double Geometry::getVolume() const
    {
        return _volume;
    }

    void Geometry::setVolume(double volume)
    {
        _volume = volume;
    }

    const BoundingBox& Geometry::getBoundingBox() const
    {
        return _boundingBox;
    }

    void Geometry::setBoundingBox(const BoundingBox& boundingBox)
    {
        _boundingBox = boundingBox;
    }

    Tessellation& Geometry::getTessellation()
    {
        return _tessellation;
    }

    const Tessellation& Geometry::getTessellation() const
    {
        return _tessellation;
    }

    bool Geometry::hasMesh() const
    {
        return !_tessellation.empty();
    }

} // namespace EgLab::GE
