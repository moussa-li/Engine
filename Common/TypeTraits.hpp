#pragma once

#include <cstddef>
#include <type_traits>

namespace EgLab::Common
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

    template <bool B, typename T, typename F>
    struct conditional
    {
        using type = T;
    };

    template <typename T, typename F>
    struct conditional<false, T, F>
    {
        using type = F;
    };

    template <bool B, typename T, typename F>
    using conditional_t = typename conditional<B, T, F>::type;

    using FalseType = integralConstant<bool, false>;
    using TrueType = integralConstant<bool, true>;

    template <typename T>
    struct isArray
    {
        static constexpr bool value = false;
    };

    template <typename T>
    struct isArray<T[]>
    {
        static constexpr bool value = true;
    };

    template <typename T, size_t N>
    struct isArray<T[N]>
    {
        static constexpr bool value = true;
    };

    template <typename T>
    constexpr bool isArray_v = isArray<T>::value;

    template <typename T>
    struct isFunction : FalseType
    {
    };

    template <typename Ret, typename... Args>
    struct isFunction<Ret(Args...)> : TrueType
    {
    };

    template <typename Ret, typename... Args>
    struct isFunction<Ret(Args..., ...)> : TrueType
    {
    };

    // cv 限定符版本
    template <typename Ret, typename... Args>
    struct isFunction<Ret(Args...) const> : TrueType
    {
    };

    template <typename Ret, typename... Args>
    struct isFunction<Ret(Args...) volatile> : TrueType
    {
    };

    template <typename Ret, typename... Args>
    struct isFunction<Ret(Args...) const volatile> : TrueType
    {
    };

    // ref 限定符版本
    template <typename Ret, typename... Args>
    struct isFunction<Ret(Args...) &> : TrueType
    {
    };

    template <typename Ret, typename... Args>
    struct isFunction<Ret(Args...) const &> : TrueType
    {
    };

    template <typename Ret, typename... Args>
    struct isFunction<Ret(Args...) volatile &> : TrueType
    {
    };

    template <typename Ret, typename... Args>
    struct isFunction<Ret(Args...) const volatile &> : TrueType
    {
    };

    template <typename Ret, typename... Args>
    struct isFunction<Ret(Args...) &&> : TrueType
    {
    };

    template <typename Ret, typename... Args>
    struct isFunction<Ret(Args...) const &&> : TrueType
    {
    };

    template <typename Ret, typename... Args>
    struct isFunction<Ret(Args...) volatile &&> : TrueType
    {
    };

    template <typename Ret, typename... Args>
    struct isFunction<Ret(Args...) const volatile &&> : TrueType
    {
    };

    template <typename T>
    constexpr bool isFunction_v = isFunction<T>::value;

    template <typename T>
    struct isReferenceable
    {
    private:
        static void test(T &&);
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
    struct RemoveRef<T &>
    {
        using type = T;
    };

    template <typename T>
    struct RemoveRef<T &&>
    {
        using type = T;
    };

    template <typename T>
    using RemoveRefT = typename RemoveRef<T>::type;

    template <typename T>
    struct removeCV
    {
        using type = T;
    };

    template <typename T>
    struct removeCV<const T>
    {
        using type = T;
    };

    template <typename T>
    struct removeCV<volatile T>
    {
        using type = T;
    };

    template <typename T>
    struct removeCV<const volatile T>
    {
        using type = T;
    };

    template <typename T>
    using removeCV_t = typename removeCV<T>::type;

    template <typename T>
    struct removeExtent
    {
        using type = T;
    };

    template <typename T>
    struct removeExtent<T[]>
    {
        using type = T;
    };

    template <typename T, size_t N>
    struct removeExtent<T[N]>
    {
        using type = T;
    };

    template <typename T>
    using removeExtent_t = typename removeExtent<T>::type;

    template <typename T>
    struct addPointer
    {
        using type = T *;
    };

    template <typename T>
    using addPointer_t = typename addPointer<T>::type;

    template <typename T, bool = isReferenceable<T>::value>
    struct addRvalueRefImpl
    {
        using type = T &&;
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

    template <typename T>
    struct decay
    {
    private:
        using U = RemoveRefT<T>;

    public:
        using type = conditional_t<isArray_v<U>, removeExtent_t<U> *,
                                   conditional_t<isFunction_v<U>, addPointer_t<U>, removeCV_t<U>>>;
    };

    template <typename T>
    using decay_t = typename decay<T>::type;

    template <bool B, typename T = void>
    struct enableIf
    {
    };

    template <typename T>
    struct enableIf<true, T>
    {
        using type = T;
    };

    template <bool B, typename T = void>
    using enableIf_t = typename enableIf<B, T>::type;

    template <typename T, typename U>
    struct isSame
    {
    public:
        static constexpr bool value = false;
    };

    template <typename T>
    struct isSame<T, T>
    {
    public:
        static constexpr bool value = true;
    };

    template <typename T, typename U>
    using isSame_v = isSame<T, T>::value;

    template <typename... Args>
    struct isOneParam
    {
        static constexpr bool value = (sizeof...(Args) == 1);
    };

    template <typename... Args>
    using isOneParam_v = typename isOneParam<Args...>::value;

    template <typename First, typename... Rest>
    struct FirstType
    {
        using type = First;
    };

    template <typename First, typename... Rest>
    using FirstType_t = FirstType<First, Rest...>::type;

    template <typename T = void>
    struct equal
    {
        constexpr bool operator()(const T &left, const T &right) const
        {
            return left == right;
        }
    };

    template <typename To>
    static TrueType testConvertible(To);

    static FalseType testConvertible(...);

    template <typename From, typename To, typename = void>
    struct isConvertible : FalseType
    {
    };

    // FIXME: isConvertible is not effect
    template <typename From, typename To>
    struct isConvertible<From, To, decltype(testConvertible<To>(std::declval<From>()))> : TrueType
    {
    };

} // namespace EgLab::Common