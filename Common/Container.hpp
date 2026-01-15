#pragma once

#include "Common/Iterator.hpp"

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

        virtual ~Container() = default;

        virtual Iterator<ContainerType> *begin() = 0;
        virtual Iterator<ContainerType> *end() = 0;
    };
} // namespace EgLab