#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "MeshEngine/Common/Definitions.hpp"

namespace EgLab::ME
{
    class Mesh;

    /// Element ids grouped by the side of a cutting plane their centroid lies on.
    struct PlaneCutResult
    {
        /// Elements whose centroid satisfies dot(normal, centroid - point) >= 0.
        Common::DynamicArray<IdType> positiveElementIds;
        /// Elements whose centroid satisfies dot(normal, centroid - point) < 0.
        Common::DynamicArray<IdType> negativeElementIds;
    };

    /// Splits the elements of a mesh into positive-side and negative-side sets using the
    /// signed distance of each element centroid to a cutting plane.
    class PlaneCut
    {
    public:
        explicit PlaneCut(Common::SharedPtr<Mesh> mesh);

        /// Classifies every element of the mesh.
        /// @param normal Plane normal; it is normalized internally.
        /// @param point  A point lying on the cutting plane.
        PlaneCutResult cut(const CoordType& normal, const CoordType& point) const;

    private:
        Common::SharedPtr<Mesh> _mesh;
    };
} // namespace EgLab::ME
