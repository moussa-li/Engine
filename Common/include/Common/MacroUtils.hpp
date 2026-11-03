#pragma once

// This file defines various macros to handle compiler differences and C++ standard features.

#define VALUE_ALIAS(x)                 \
public:                                \
    using ValueType = x;               \
    using ValuePtr = x*;               \
    using ValueCPtr = const x*;        \
    using ValueCPtrC = const x* const; \
    using ValueRef = x&;               \
    using ValueCRef = const x&;