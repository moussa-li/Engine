#pragma once

namespace EgLab::Platform
{
    /// Surface-UV display applied to imported B-Rep geometry faces.
    ///
    /// Each render node carries the UV taken straight from the face's surface parameterisation;
    /// the dedicated UV shader interpolates it across the triangles and colours the faces from
    /// the interpolated UV. Geometry models only; regular meshes keep their shaded appearance.
    struct GeometryUv
    {
        bool enabled{false};
    };
} // namespace EgLab::Platform
