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

    template <typename T, T v>
    struct integralConstant
    {
        static constexpr T value = v;

        using ValueType = T;
        using Type = integralConstant;

        constexpr operator ValueType() const noexcept
        {
            return value;
        }

        constexpr ValueType operator()() const noexcept
        {
            return value;
        }
    };

    using FalseType = integralConstant<bool, false>;
    using TrueType = integralConstant<bool, true>;

    template <typename T>
    struct isReferenceable
    {
    private:
        static void test(T&&);
        template <typename U>
        static auto check(int) -> decltype(test(declval<U>()), TrueType{});
        template <typename>
        static FalseType check(...);

    public:
        static constexpr bool value = decltype(check<T>(0))::value;
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

    template <typename T, bool = isReferenceable<T>::value>
    struct addRvalueRefImpl
    {
        using type = T&&;
    };

    template <typename T>
    struct addRvalueRefImpl<T, false>
    {
        using type = T;
    };

    template <typename T>
    struct addRvalueRef : addRvalueRefImpl<T>
    {
    };

    template <>
    struct addRvalueRef<void>
    {
        using type = void;
    };

    template <>
    struct addRvalueRef<const void>
    {
        using type = const void;
    };

    template <>
    struct addRvalueRef<volatile void>
    {
        using type = volatile void;
    };

    template <>
    struct addRvalueRef<const volatile void>
    {
        using type = const volatile void;
    };

    template <typename T>
    typename addRvalueRef<T>::type declval() noexcept;

    template <typename T>
    using void_t = void;

} // namespace EgLab