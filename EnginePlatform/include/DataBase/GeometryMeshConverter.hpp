#pragma once

#include <cstdint>

#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "DataBase/GeometryData.hpp"

namespace EgLab::GE
{
    class GeometryDocument;
}

namespace EgLab::ME
{
    class Mesh;
}

namespace EgLab::Platform
{
    /// Converts a GeometryEngine document into a renderable ME::Mesh (Tri3) plus the
    /// elemId -> feature table consumed by GeometryData.
    class GeometryMeshConverter
    {
    public:
        static Common::Return convert(const Common::SharedPtr<GE::GeometryDocument>& document,
                                      Common::SharedPtr<ME::Mesh>& outMesh,
                                      Common::DynamicArray<GeometryFeatureRef>& outElementFeatures,
                                      uint32_t& outTriangleCount);
    };

} // namespace EgLab::Platform
