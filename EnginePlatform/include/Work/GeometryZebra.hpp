#pragma once

#include "Common/Vector.hpp"

namespace EgLab::Platform
{
    /// Zebra (isophote) stripe analysis applied to imported B-Rep geometry faces.
    ///
    /// The stripes are produced by the dedicated zebra shader from a user-configured virtual
    /// light direction, so a flat region keeps one band while steep normal changes bend it.
    /// The pattern is independent of the scene camera and the global lighting. Geometry models
    /// only; the regular meshes keep their shaded appearance.
    struct GeometryZebra
    {
        static constexpr float MIN_FREQUENCY = 2.0f;
        static constexpr float MAX_FREQUENCY = 40.0f;
        static constexpr float DEFAULT_FREQUENCY = 8.0f;

        bool enabled{false};
        float frequency{DEFAULT_FREQUENCY};
        /// Direction towards the virtual zebra light, in model space. The stripes mirror it
        /// instead of the camera eye ray, so moving the camera never changes the pattern.
        Common::Vector3f lightDirection{0.0f, 0.0f, 1.0f};

        /// Keeps a value coming from the UI or an IPC client inside the supported range.
        /// Non-finite and non-positive values fall back to the default density.
        static float clampFrequency(float value)
        {
            if (!(value > 0.0f))
            {
                return DEFAULT_FREQUENCY;
            }
            if (value < MIN_FREQUENCY)
            {
                return MIN_FREQUENCY;
            }
            if (value > MAX_FREQUENCY)
            {
                return MAX_FREQUENCY;
            }
            return value;
        }

        /// Normalises the zebra light direction; a degenerate (zero/non-finite) direction
        /// falls back to the default so the bands never collapse into a single value.
        static Common::Vector3f clampDirection(const Common::Vector3f &value)
        {
            Common::Vector3f direction = value;
            const float length2 = direction.x() * direction.x() +
                                  direction.y() * direction.y() +
                                  direction.z() * direction.z();
            if (!(length2 > 1e-12f))
            {
                return Common::Vector3f(0.0f, 0.0f, 1.0f);
            }
            direction.normalize();
            return direction;
        }

        /// Color of the dark bands; the light bands stay white.
        static Common::Vector4f darkColor()
        {
            return Common::Vector4f(0.05f, 0.05f, 0.05f, 1.0f);
        }
    };

} // namespace EgLab::Platform
