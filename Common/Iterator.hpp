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

    template <class Container>
    class Iterator
    {
        using ValueType = TypeTrait<Container>::ValueType;
        using ValuePtr = TypeTrait<Container>::ValuePtr;
        using ValueCPtr = TypeTrait<Container>::ValueCPtr;
        using ValueCPtrC = TypeTrait<Container>::ValueCPtrC;
        using ValueRef = TypeTrait<Container>::ValueRef;
        using ValueCRef = TypeTrait<Container>::ValueCRef;

        using ContainerType = Container;
        using ContainerPtr = Container*;
        using ContinerCPtr = const Container*;
        using ContainerCPtrC = const Container* const;
        using ContainerRef = Container&;
        using ContainerCRef = const Container&;

    public:
        virtual bool hasNext() const = 0;
        virtual ValueRef next() = 0;

        virtual ValueRef operator*() = 0;

        virtual ValueCRef operator*() const = 0;

        virtual ~Iterator() = default;

    private:
    };

} // namespace EgLab