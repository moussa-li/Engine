#pragma once
#include "Common/Vector.hpp"

namespace EgLab::RE
{
    /// Quaternion with scalar-first component order (w, x, y, z).
    class Quatf : public Common::Vector4f
    {
    public:
        /**
         * @brief Constructs a quaternion from its scalar and vector components.
         * @param w Scalar component.
         * @param x X component of the vector part.
         * @param y Y component of the vector part.
         * @param z Z component of the vector part.
         */
        Quatf(float w, float x, float y, float z) : Common::Vector4f(w, x, y, z)
        {
        }
        /**
         * @brief Creates a quaternion representing a rotation around an axis.
         * @param axis Rotation axis; it is normalized before use.
         * @param angleRad Rotation angle in radians.
         * @return The corresponding rotation quaternion.
         */
        static Quatf fromAxisAndAngle(const Common::Vector3f& axis, float angleRad)
        {
            float halfAngle = angleRad * float(0.5);
            float s = std::sin(halfAngle);
            float c = std::cos(halfAngle);

            Common::Vector3f normalizedAxis = axis;
            normalizedAxis.normalize();

            return Quatf(c, normalizedAxis[0] * s, normalizedAxis[1] * s, normalizedAxis[2] * s);
        }

        // const float& operator[](size_t dim) const
        // {
        //     return this->_data[0][dim];
        // }

        /**
         * @brief Multiplies this quaternion by another using the Hamilton product.
         * @param q Right-hand quaternion.
         * @return The product quaternion; multiplication is not commutative.
         */
        Quatf operator*(const Quatf& q) const
        {
            const float &w1 = (*this)[0], x1 = (*this)[1], y1 = (*this)[2], z1 = (*this)[3];
            const float &w2 = q[0], x2 = q[1], y2 = q[2], z2 = q[3];

            return Quatf(
                w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2, w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2,
                w1 * y2 - x1 * z2 + y1 * w2 + z1 * x2, w1 * z2 + x1 * y2 - y1 * x2 + z1 * w2);
        }

        /**
         * @brief Rotates a three-dimensional vector by this quaternion.
         * @param v Vector to rotate.
         * @return The rotated vector.
         */
        Common::Vector3f rotateVector(const Common::Vector3f& v) const
        {
            // Extract the vector part of the quaternion (x, y, z).
            Common::Vector3f u((*this)[1], (*this)[2], (*this)[3]);

            // Expanded form: v + 2.0 * (u x (u x v + w * v)).
            // This avoids converting the quaternion to a rotation matrix.
            Common::Vector3f uv = u.cross(v);
            Common::Vector3f uuv = u.cross(uv);

            return v + ((uv * (*this)[0]) + uuv) * float(2.0);
        }
    };
} // namespace EgLab::RE