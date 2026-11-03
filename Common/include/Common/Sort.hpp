#pragma once

#include <Common/Utils.hpp>

#include "Common/DynamicArray.hpp"

namespace EgLab::Common
{
    template <typename BASIC_TYPE>
    void sort3(BASIC_TYPE& d1, BASIC_TYPE& d2, BASIC_TYPE& d3)
    {
        if (d1 > d2)
        {
            Common::swap<BASIC_TYPE>(d1, d2);
        }
        if (d2 > d3)
        {
            Common::swap<BASIC_TYPE>(d2, d3);
            if (d1 > d2)
            {
                Common::swap<BASIC_TYPE>(d1, d2);
            }
        }
    }

    template <typename T>
    int partition(DynamicArray<T>& arr, int low, int high)
    {
        T pivot = arr[high];
        int i = low - 1;
        for (int j = low; j < high; ++j)
        {
            if (arr[j] <= pivot)
            {
                ++i;
                std::swap(arr[i], arr[j]);
            }
        }
        std::swap(arr[i + 1], arr[high]);
        return i + 1;
    }

    template <typename T>
    void quickSortImpl(DynamicArray<T>& arr, int low, int high)
    {
        if (low < high)
        {
            int pi = partition(arr, low, high);
            quickSortImpl(arr, low, pi - 1);
            quickSortImpl(arr, pi + 1, high);
        }
    }

    template <typename T>
    void sort(DynamicArray<T>& data)
    {
        if (data.size() > 1)
        {
            quickSortImpl(data, 0, static_cast<int>(data.size()) - 1);
        }
    }

} // namespace EgLab::Common
