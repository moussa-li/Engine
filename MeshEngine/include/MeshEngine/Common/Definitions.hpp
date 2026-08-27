#pragma once

#include "Common/Vector.hpp"

namespace EgLab::ME
{
    using DeltaTime = float;

    using IdType = unsigned int;

    using Scalar = double;

    using CoordType = Common::Vector<Scalar, 3>;

    using IdxType = unsigned int;

    constexpr IdType MAX_ID = UINT_MAX;

    constexpr IdxType INVALID_IDX = UINT_MAX;

} // namespace EgLab::ME