#pragma once

#include <cstddef>

// #include "Utils.hpp"

namespace EgLab
{
    // 存储第 Index 个元素的类模板
    template <size_t Index, typename T>
    struct TupleElement
    {
        T value;

        TupleElement() = default;

        template <typename U>
        TupleElement(U&& v) : value(forward<U>(v))
        {
        }
    };

    // 递归继承实现 TupleImpl
    template <size_t Index, typename... Ts>
    struct TupleImpl;

    // 递归终止条件：没有元素
    template <size_t Index>
    struct TupleImpl<Index>
    {
    };

    // 递归继承：拆分第一个元素和剩余元素
    template <size_t Index, typename T, typename... Ts>
    struct TupleImpl<Index, T, Ts...> : TupleElement<Index, T>, TupleImpl<Index + 1, Ts...>
    {
        TupleImpl() = default;

        template <typename U, typename... Us>
        TupleImpl(U&& v, Us&&... vs)
            : TupleElement<Index, T>(forward<U>(v)), TupleImpl<Index + 1, Ts...>(forward<Us>(vs)...)
        {
        }
    };

    // Tuple 主类模板
    template <typename... Ts>
    struct Tuple : TupleImpl<0, Ts...>
    {
        using base_type = TupleImpl<0, Ts...>;

        Tuple() = default;

        template <typename... Us>
        Tuple(Us&&... vs) : base_type(forward<Us>(vs)...)
        {
        }
    };

    // 辅助模板，获取 Tuple 中第 I 个类型
    template <size_t I, typename TupleType>
    struct TupleElementType;

    template <size_t I, typename T, typename... Ts>
    struct TupleElementType<I, Tuple<T, Ts...>> : TupleElementType<I - 1, Tuple<Ts...>>
    {
    };

    template <typename T, typename... Ts>
    struct TupleElementType<0, Tuple<T, Ts...>>
    {
        using type = T;
    };

    // get 函数，获取第 I 个元素引用
    template <size_t I, typename... Ts>
    auto& get(Tuple<Ts...>& t)
    {
        return static_cast<TupleElement<I, typename TupleElementType<I, Tuple<Ts...>>::type>&>(t)
            .value;
    }

} // namespace EgLab