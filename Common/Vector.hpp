#pragma once

#include <math.h>

#include <cstddef>

#include "Matrix.hpp"

namespace EgLab::Common
{
    template <typename Scalar, size_t Dim>
    class Vector : public Matrix<Scalar, 1, Dim>
    {
    public:
        Vector() = default;

        Vector(const Vector &other) = default;

        Vector(Vector &&other) noexcept = default;

        Vector &operator=(const Vector &other) = default;

        Vector &operator=(Vector &&other) noexcept = default;

        template <typename... Args,
                  typename = std::enable_if_t<sizeof...(Args) == Dim &&
                                              (std::is_convertible_v<Args, Scalar> && ...)>>
        Vector(Args... args)
        {
            Scalar temp[] = {static_cast<Scalar>(args)...};
            for (size_t i = 0; i < Dim; ++i) this->_data[0][i] = temp[i];
        }

        Scalar &operator[](size_t dim)
        {
            return this->_data[0][dim];
        }

        Scalar &x()
        {
            return this->_data[0][0];
        }

        Scalar &y()
        {
            return this->_data[0][1];
        }

        Scalar &z()
        {
            if constexpr (Dim >= 3)
            {
                return this->_data[0][2];
            }
            else
            {
                static_assert(Dim >= 3, "Cannot call z() on a 2-dim vector!");
            }
        }

        const Scalar &x() const
        {
            return this->_data[0][0];
        }

        const Scalar &y() const
        {
            return this->_data[0][1];
        }

        const Scalar &z() const
        {
            if constexpr (Dim >= 3)
            {
                return this->_data[0][2];
            }
            else
            {
                static_assert(Dim >= 3, "Cannot call z() on a 2-dim vector!");
            }
        }

        Vector<Scalar, Dim> cross(const Vector<Scalar, Dim> &other) const
        {
            Vector<Scalar, Dim> res;

            res[0] = this->y() * other.z() - this->z() * other.y();
            res[1] = this->z() * other.x() - this->x() * other.z();
            res[2] = this->x() * other.y() - this->y() * other.x();

            return res;
        }

        Scalar length() const
        {
            return std::sqrt(length2());
        }

        Scalar length2() const
        {
            return this->_data[0][0] * this->_data[0][0] + this->_data[0][1] * this->_data[0][1] +
                   this->_data[0][2] * this->_data[0][2];
        }

        void normalize()
        {
            Scalar l = length();
            if (l == 0) return;
            this->_data[0][0] /= l;
            this->_data[0][1] /= l;
            this->_data[0][2] /= l;
        }

        Scalar dot(const Vector<Scalar, Dim> &other) const
        {
            return this->x() * other.x() + this->y() * other.y() + this->z() * other.z();
        }

        Vector operator+(const Vector &other) const
        {
            Vector result;
            for (int i = 0; i < Dim; ++i)
            {
                result._data[0][i] = this->_data[0][i] + other._data[0][i];
            }
            return result;
        }

        Vector operator-(const Vector &other) const
        {
            Vector result;
            for (int i = 0; i < Dim; ++i)
            {
                result._data[0][i] = this->_data[0][i] - other._data[0][i];
            }
            return result;
        }

        Vector operator*(Scalar scalar) const
        {
            Vector result;
            for (int i = 0; i < Dim; ++i)
            {
                result._data[0][i] = this->_data[0][i] * scalar;
            }
            return result;
        }

        friend Vector operator*(Scalar scalar, const Vector &vec)
        {
            return vec * scalar;
        }
    };

    typedef Vector<double, 4> Vector4d;
    typedef Vector<float, 4> Vector4f;

    typedef Vector<double, 3> Vector3d;
    typedef Vector<float, 3> Vector3f;

    typedef Vector<double, 2> Vector2d;
    typedef Vector<float, 2> Vector2f;

} // namespace EgLab::Common