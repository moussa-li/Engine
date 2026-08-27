#pragma once

#include "Common/TypeTraits.hpp"

namespace EgLab::Common
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

    template <class T>
    inline const T min(const T& a, const T& b)
    {
        return (b < a) ? b : a;
    }
    template <class T>
    inline const T max(const T& a, const T& b)
    {
        return (b > a) ? b : a;
    }

    template <typename T>
    inline void swap(T& a, T& b)
    {
        T temp = move(a);
        a = move(b);
        b = move(temp);
    }

    template <typename T, typename = void>
    struct hashFun;
} // namespace EgLab::Common
