#pragma once

#include <cstdint>

#include "GeometryEngine/Common/GeometryAPI.hpp"
#include "GeometryEngine/Common/GeometryTypes.hpp"
#include "GeometryEngine/Geometry/Tessellation.hpp"

namespace EgLab::GE
{
    enum class GeometryType : std::uint8_t
    {
        Unknown = 0,
        Point,
        Curve,
        Surface,
        Solid
    };

    /// Geometric data attached to one topological feature.
    ///
    /// Measures use the model unit: length in unit, area in unit^2 and volume in unit^3.
    class GeometryEngineAPI Geometry
    {
    public:
        Geometry();

        explicit Geometry(GeometryType type);

        GeometryType getType() const;

        void setType(GeometryType type);

        bool isValid() const;

        double getLength() const;

        void setLength(double length);

        double getArea() const;

        void setArea(double area);

        double getVolume() const;

        void setVolume(double volume);

        const BoundingBox& getBoundingBox() const;

        void setBoundingBox(const BoundingBox& boundingBox);

        Tessellation& getTessellation();

        const Tessellation& getTessellation() const;

        bool hasMesh() const;

    private:
        GeometryType _type{GeometryType::Unknown};
        double _length{0.0};
        double _area{0.0};
        double _volume{0.0};
        BoundingBox _boundingBox;
        Tessellation _tessellation;
    };

} // namespace EgLab::GE
