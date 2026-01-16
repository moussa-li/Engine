#pragma once

namespace EgLab
{
    template <class T>
    struct TypeTrait
    {
        using ValueType = typename T::ValueType;
        using ValuePtr = typename T::ValuePtr;
        using ValueCPtr = typename T::ValueCPtr;
        using ValueCPtrC = typename T::ValueCPtrC;
        using ValueRef = typename T::ValueRef;
        using ValueCRef = typename T::ValueCRef;
    };

    template <typename T>
    struct RemoveRef
    {
        using type = T;
    };

    template <typename T>
    struct RemoveRef<T&>
    {
        using type = T;
    };

    template <typename T>
    struct RemoveRef<T&&>
    {
        using type = T;
    };

    template <typename T>
    using RemoveRefT = typename RemoveRef<T>::type;
} // namespace EgLab