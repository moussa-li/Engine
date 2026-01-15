#pragma once

#include <cstddef>

namespace EgLab
{
    template <typename Scalar, size_t Col, size_t Row>
    class Matrix
    {
    public:
        Matrix()
        {
        }

        template <typename... Args>
        Matrix(Args... args)
        {
            static_assert(sizeof...(Args) == Col * Row,
                          "number of parameter must be equal with col * row");
            Scalar temp[] = {static_cast<Scalar>(args)...};
            for (size_t i = 0; i < Col * Row; ++i) _data[i / Row][i % Row] = temp[i];
        }

        Scalar* operator[](size_t i)
        {
            return _data[i];
        }

    protected:
        Scalar _data[Col][Row];
    };

    typedef Matrix<float, 4, 4> Matrix4f;
    typedef Matrix<float, 3, 3> Matrix3f;
    typedef Matrix<double, 4, 4> Matrix4d;
    typedef Matrix<double, 3, 3> Matrix3d;
} // namespace EgLab