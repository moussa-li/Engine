#pragma once

#include "Common/Vector.hpp"
#include "Definites.hpp"

namespace EgLab::RE
{
    /// Stores translation, rotation, and scale for a renderable object.
    class Transform
    {
    public:
        /// @brief Creates a transform with default components.
        Transform();

        /// @brief Destroys the transform.
        ~Transform() = default;

        /**
         * @brief Creates a transform from position, rotation, and scale.
         * @param position Translation component.
         * @param rotation Rotation component.
         * @param scale Scale component.
         */
        Transform(CoordType position, CoordType rotation, CoordType scale);

        /// @brief Copies the transform components from another transform.
        /// @param other Transform to copy.
        Transform(const Transform &other);

        /// @brief Builds the combined transform matrix.
        Common::Matrix4f getMatrix();
        /// @brief Builds the rotation matrix.
        Common::Matrix4f getRotationMatrix();

        /// @brief Sets the translation component.
        /// @param positoin New position.
        void setPosition(CoordType positoin);

    private:
        CoordType _position;
        CoordType _rotation;
        CoordType _scale;
    };

} // namespace EgLab::RE
