#pragma once

#include <cstddef>

#include <Common/Utils.hpp>

#include "Common/DynamicArray.hpp"

namespace EgLab::Common
{
    template <typename BASIC_TYPE>
    void sort3(BASIC_TYPE& d1, BASIC_TYPE& d2, BASIC_TYPE& d3)
    {
        if (d1 > d2)
        {
            Common::swapValues<BASIC_TYPE>(d1, d2);
        }
        if (d2 > d3)
        {
            Common::swapValues<BASIC_TYPE>(d2, d3);
            if (d1 > d2)
            {
                Common::swapValues<BASIC_TYPE>(d1, d2);
            }
        }
    }

    /// Utility helpers for the introsort below. They are stateless implementation details and
    /// rely on the caller supplied strict weak ordering (operator<).
    namespace SortDetail
    {
        constexpr std::size_t insertionSortThreshold = 16;

        template <typename T>
        void exchange(DynamicArray<T>& arr, std::size_t lhs, std::size_t rhs)
        {
            if (lhs != rhs)
            {
                Common::swapValues(arr[lhs], arr[rhs]);
            }
        }

        template <typename T>
        void insertionSort(DynamicArray<T>& arr, std::size_t first, std::size_t last)
        {
            for (std::size_t index = first + 1; index < last; ++index)
            {
                T value = Common::move(arr[index]);
                std::size_t hole = index;
                while (hole > first && value < arr[hole - 1])
                {
                    arr[hole] = Common::move(arr[hole - 1]);
                    --hole;
                }
                arr[hole] = Common::move(value);
            }
        }

        template <typename T>
        void siftDown(DynamicArray<T>& arr, std::size_t first, std::size_t root,
                      std::size_t last)
        {
            while (true)
            {
                std::size_t child = first + (root - first) * 2 + 1;
                if (child >= last)
                {
                    return;
                }
                if (child + 1 < last && arr[child] < arr[child + 1])
                {
                    ++child;
                }
                if (!(arr[root] < arr[child]))
                {
                    return;
                }
                exchange(arr, root, child);
                root = child;
            }
        }

        template <typename T>
        void heapSort(DynamicArray<T>& arr, std::size_t first, std::size_t last)
        {
            const std::size_t count = last - first;
            for (std::size_t parent = count / 2; parent > 0; --parent)
            {
                siftDown(arr, first, first + parent - 1, last);
            }
            for (std::size_t end = last; end > first + 1; --end)
            {
                exchange(arr, first, end - 1);
                siftDown(arr, first, first, end - 1);
            }
        }

        /// Sorts the first, middle and last element and returns the median value. A median
        /// pivot keeps already sorted input from degrading to the quadratic worst case.
        template <typename T>
        T choosePivot(DynamicArray<T>& arr, std::size_t first, std::size_t last)
        {
            const std::size_t middle = first + (last - first) / 2;
            const std::size_t final = last - 1;
            if (arr[middle] < arr[first])
            {
                exchange(arr, middle, first);
            }
            if (arr[final] < arr[first])
            {
                exchange(arr, final, first);
            }
            if (arr[final] < arr[middle])
            {
                exchange(arr, final, middle);
            }
            return arr[middle];
        }

        template <typename T>
        void introSortLoop(DynamicArray<T>& arr, std::size_t first, std::size_t last,
                           std::size_t depthLimit)
        {
            while (last - first > insertionSortThreshold)
            {
                if (depthLimit == 0)
                {
                    heapSort(arr, first, last);
                    return;
                }
                --depthLimit;

                const T pivot = choosePivot(arr, first, last);
                std::size_t less = first;
                std::size_t greater = last;
                std::size_t index = first;
                while (index < greater)
                {
                    if (arr[index] < pivot)
                    {
                        exchange(arr, index, less);
                        ++less;
                        ++index;
                    }
                    else if (pivot < arr[index])
                    {
                        --greater;
                        exchange(arr, index, greater);
                    }
                    else
                    {
                        ++index;
                    }
                }

                // Recurse into the smaller partition and iterate over the larger one so the
                // explicit recursion depth stays logarithmic even for skewed splits.
                if (less - first < last - greater)
                {
                    introSortLoop(arr, first, less, depthLimit);
                    first = greater;
                }
                else
                {
                    introSortLoop(arr, greater, last, depthLimit);
                    last = less;
                }
            }
            insertionSort(arr, first, last);
        }

        inline std::size_t computeDepthLimit(std::size_t count)
        {
            std::size_t depth = 0;
            while (count > 1)
            {
                count >>= 1;
                ++depth;
            }
            return depth * 2;
        }
    } // namespace SortDetail

    template <typename T>
    void sort(DynamicArray<T>& data)
    {
        if (data.size() > 1)
        {
            SortDetail::introSortLoop(data, 0, data.size(),
                                      SortDetail::computeDepthLimit(data.size()));
        }
    }

} // namespace EgLab::Common
