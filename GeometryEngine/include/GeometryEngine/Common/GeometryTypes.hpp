#pragma once

#include <cstddef>
#include <cstdint>

#include "Common/BBox.hpp"
#include "Common/Vector.hpp"

namespace EgLab::GE
{
    using Scalar = double;

    using Vec3 = Common::Vector<Scalar, 3>;

    using BoundingBox = Common::BBox<Scalar, 3>;

    using IdType = std::uint32_t;

    using IdxType = std::uint32_t;

    constexpr IdType INVALID_ID = 0xFFFFFFFFu;

    constexpr IdxType INVALID_IDX = 0xFFFFFFFFu;

} // namespace EgLab::GE
