#pragma once

#include "Common/Iterator.hpp"
#include "Common/UniquePtr.hpp"

namespace EgLab
{
    template <class T>
    class Container
    {
    public:
        // using ValueType = TypeTrait<T>::ValueType;
        // using ValuePtr = TypeTrait<T>::ValuePtr;
        // using ValueCPtr = TypeTrait<T>::ValueCPtr;
        // using ValueCPtrC = TypeTrait<T>::ValueCPtrC;
        // using ValueRef = TypeTrait<T>::ValueRef;
        // using ValueCRef = TypeTrait<T>::ValueCRef;

        using ContainerType = T;
        using ContainerPtr = T *;
        using ContainerCPtr = const T *;
        using ContainerCPtrC = const T *const;
        using ContainerRef = T &;
        using ContainerCRef = const T &;
        using IteratorT = Iterator<T>;
        using CIteratorT = CIterator<T>;

        virtual ~Container() = default;

        virtual IteratorT begin() = 0;

        virtual CIteratorT begin() const = 0;

        virtual IteratorT end() = 0;

        virtual CIteratorT end() const = 0;
    };
} // namespace EgLab