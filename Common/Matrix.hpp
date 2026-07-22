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

        static Matrix<Scalar, Col, Row> Identity()
        {
            Matrix<Scalar, Col, Row> res;
            for (int i = 0; i < Col; i++)
            {
                res[i][i] = 1;
            }
            return res;
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

        const Scalar* operator[](size_t i) const
        {
            return _data[i];
        }

        template <size_t OtherCol>
        Matrix<Scalar, OtherCol, Row> operator*(const Matrix<Scalar, OtherCol, Row>& other) const
        {
            static_assert(OtherCol == Row,
                          "Matrix dimension mismatch: A's columns must equal B's rows!");

            Matrix<Scalar, OtherCol, Row> result;
            for (int r = 0; r < Row; ++r)
            {
                for (int c = 0; c < OtherCol; ++c)
                {
                    Scalar sum = static_cast<Scalar>(0);
                    for (int k = 0; k < Col; ++k)
                    {
                        sum += _data[k][r] * other._data[c][k];
                    }
                    result._data[c][r] = sum;
                }
            }

            return result;
        }

        Matrix<Scalar, Col, Row> operator*(Scalar rat) const
        {
            Matrix<Scalar, Col, Row> result;
            for (int c = 0; c < Col; ++c)
            {
                for (int r = 0; r < Row; ++r)
                {
                    result._data[c][r] = _data[c][r] * rat;
                }
            }
            return result;
        }

        Matrix<Scalar, Col, Row> operator-(const Matrix<Scalar, Col, Row>& other) const
        {
            Matrix<Scalar, Col, Row> result;
            for (int c = 0; c < Col; ++c)
            {
                for (int r = 0; r < Row; ++r)
                {
                    result._data[c][r] = _data[c][r] - other._data[c][r];
                }
            }
            return result;
        }

        const Scalar* data() const
        {
            return &_data[0][0];
        }

    protected:
        Scalar _data[Col][Row] = {};
    };

    typedef Matrix<float, 4, 4> Matrix4f;
    typedef Matrix<float, 3, 3> Matrix3f;
    typedef Matrix<double, 4, 4> Matrix4d;
    typedef Matrix<double, 3, 3> Matrix3d;

} // namespace EgLab

#include <iomanip>
#include <iostream>

namespace EgLab
{
    template <typename Scalar, size_t Col, size_t Row>
    std::ostream& operator<<(std::ostream& os, const Matrix<Scalar, Col, Row>& mat)
    {
        os << "\n" << "Matrix<" << Col << "x" << Row << ">:\n";

        std::ios oldState(nullptr);
        oldState.copyfmt(os);

        os << std::defaultfloat << std::setprecision(4);

        for (int i = 0; i < Col; ++i)
        {
            os << "[ ";
            for (int j = 0; j < Row; ++j)
            {
                os << std::setw(12) << static_cast<Scalar>(mat[i][j]) << " ";
            }
            os << "]\n";
        }

        os.copyfmt(oldState);

        return os;
    }
} // namespace EgLab