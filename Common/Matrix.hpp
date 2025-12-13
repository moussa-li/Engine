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

    private:
        template <typename T, typename... Rest>
        void assignData(size_t index, T value, Rest... rest)
        {
            _data[index / Row][index % Row] = value;
            assignData(index + 1, rest...);
        }

        void assignData(size_t)
        {
        }

    private:
        Scalar _data[Col][Row];
    };
} // namespace EgLab