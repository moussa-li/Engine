#pragma once

#include "Common/Vector.hpp"

namespace EgLab::RE
{
    /// Elapsed time in seconds, or the time unit supplied by the caller.
    using DeltaTime = float;

    /// Identifier type used by render-engine entities.
    using IdType = unsigned int;

    /// Scalar numeric type used by rendering math.
    using Scalar = float;

    /// Three-dimensional position or direction vector.
    using CoordType = Common::Vector3f;

    /// Two-dimensional texture coordinate.
    using TexCoordType = Common::Vector2f;

    /// Index type used for mesh and primitive data.
    using IdxType = unsigned int;

} // namespace EgLab::RE