#pragma once

#include <cstdint>

#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "DataBase/GeometryData.hpp"
#include "DataBase/GeometryMeshBuild.hpp"

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
    /// feature tables consumed by GeometryData: face elements, edge segments and vertices.
    class GeometryMeshConverter
    {
    public:
        static Common::Return convert(const Common::SharedPtr<GE::GeometryDocument>& document,
                                      GeometryMeshBuild& outBuild);
    };

} // namespace EgLab::Platform
