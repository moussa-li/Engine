#pragma once

#include "Common/TypeTraits.hpp"

namespace EgLab
{
    template <typename T>
    T&& forward(typename RemoveRef<T>::type& arg)
    {
        return static_cast<T&&>(arg);
    }

    template <typename T>
    typename RemoveRef<T>::type&& move(T&& t) noexcept
    {
        return static_cast<typename RemoveRef<T>::type&&>(t);
    }

    template <typename T>
    void swap(T& a, T& b)
    {
        T temp = move(a);
        a = move(b);
        b = move(temp);
    }
} // namespace EgLab
