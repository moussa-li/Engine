#pragma once

#include <cstddef>

#include "Matrix.hpp"

namespace EgLab
{
    template <typename Scalar, size_t Dim>
    class Vector : public Matrix<Scalar, 1, Dim>
    {
    public:
        Vector()
        {
        }

        template <typename... Args>
        Vector(Args... args)
        {
            static_assert(sizeof...(Args) == Dim, "number of parameter must be equal wtih dim");
            Scalar temp[] = {static_cast<Scalar>(args)...};
            for (size_t i = 0; i < Dim; ++i) this->_data[0][i] = temp[i];
        }
    };

    typedef Vector<double, 4> Vector4d;
    typedef Vector<float, 4> Vector4f;

    typedef Vector<double, 3> Vector3d;
    typedef Vector<float, 3> Vector3f;

    typedef Vector<double, 2> Vector2d;
    typedef Vector<float, 2> Vector2f;

} // namespace EgLab