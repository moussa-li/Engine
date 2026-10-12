#pragma once
#include "Common/DynamicArray.hpp"
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

        DynamicArray<BBox> split(const BBox& other) const
        {
            DynamicArray<BBox> pieces;
            if (_empty) return pieces;
            if (other._empty)
            {
                BBox unchanged = *this;
                pieces.pushBack(unchanged);
                return pieces;
            }

            Vector<Scalar, Dim> intersectionMin;
            Vector<Scalar, Dim> intersectionMax;
            for (size_t i = 0; i < Dim; ++i)
            {
                intersectionMin[i] = _min[i] > other._min[i] ? _min[i] : other._min[i];
                intersectionMax[i] = _max[i] < other._max[i] ? _max[i] : other._max[i];
                if (intersectionMin[i] >= intersectionMax[i])
                {
                    BBox unchanged = *this;
                    pieces.pushBack(unchanged);
                    return pieces;
                }
            }

            auto remainingMin = _min;
            auto remainingMax = _max;
            for (size_t i = 0; i < Dim; ++i)
            {
                if (remainingMin[i] < intersectionMin[i])
                {
                    auto pieceMax = remainingMax;
                    pieceMax[i] = intersectionMin[i];
                    BBox piece;
                    piece.addPoint(remainingMin);
                    piece.addPoint(pieceMax);
                    pieces.pushBack(piece);
                    remainingMin[i] = intersectionMin[i];
                }

                if (intersectionMax[i] < remainingMax[i])
                {
                    auto pieceMin = remainingMin;
                    pieceMin[i] = intersectionMax[i];
                    BBox piece;
                    piece.addPoint(pieceMin);
                    piece.addPoint(remainingMax);
                    pieces.pushBack(piece);
                    remainingMax[i] = intersectionMax[i];
                }
            }

            return pieces;
        }

    private:
        Vector<Scalar, Dim> _min;
        Vector<Scalar, Dim> _max;
        bool _empty = true;
    };

} // namespace EgLab::Common
