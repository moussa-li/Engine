#pragma once
#include "Common/Vector.hpp"

namespace EgLab::Common
{

    template <typename Scalar, size_t Dim>
    class BBox
    {
    public:
        BBox() = default;
        ~BBox() = default;

        bool isEmpty() const
        {
            return _empty;
        }

        void reset()
        {
            _empty = true;
            _min = Vector<Scalar, Dim>();
            _max = Vector<Scalar, Dim>();
        }

        Vector<Scalar, Dim> min() const
        {
            return Vector<Scalar, Dim>(_min);
        }

        Vector<Scalar, Dim> max() const
        {
            return Vector<Scalar, Dim>(_max);
        }

        bool contains(const Vector<Scalar, Dim>& point) const
        {
            if (_empty) return false;

            for (size_t i = 0; i < Dim; ++i)
            {
                if (point[i] < _min[i] || point[i] > _max[i]) return false;
            }
            return true;
        }

        void addPoint(const Vector<Scalar, Dim>& point)
        {
            if (_empty)
            {
                _min = point;
                _max = point;
                _empty = false;
            }
            else
            {
                for (size_t i = 0; i < Dim; ++i)
                {
                    if (point[i] < _min[i]) _min[i] = point[i];
                    if (point[i] > _max[i]) _max[i] = point[i];
                }
            }
        }

        void merge(const BBox& other)
        {
            if (other.isEmpty()) return;

            addPoint(other.min());
            addPoint(other.max());
        }

    private:
        Vector<Scalar, Dim> _min;
        Vector<Scalar, Dim> _max;
        bool _empty = true;
    };

} // namespace EgLab::Common
