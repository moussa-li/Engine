#pragma once

/**
 * @file String.hpp
 * @author Moussa-Li
 * @brief a quickly and lower memory string
 * @date 2025-09-28
 */

#include <cstddef>
#include <iostream>

#include "Common/CommonAPI.hpp"

namespace EgLab::Common
{
    using Length_t = size_t;
    using Capacity_t = size_t;

    constexpr size_t bufferSize = sizeof(Length_t) + sizeof(Capacity_t) + sizeof(char*);

    class CommonAPI String
    {
    public:
        String();

        String(const char* str);

        String(const String&);

        String(String&&);

        String(const unsigned char str[], const unsigned long size);

        String(const char* str, const unsigned long size);

        size_t size() const;

        const char* c_str() const;

        void append(const char* str);

        String& operator=(const String& other);

        String operator+(const String& other) const;

        String& operator+=(const String& other);

        bool operator==(const String& other) const;

        bool operator==(const char* other) const;

        char* begin()
        {
            if (is_sso)
                return sso_buffer;
            else
                return heap._data;
        }

        char* end()
        {
            if (is_sso)
                return sso_buffer + size();
            else
                return heap._data + heap.length;
        }

        const char* begin() const
        {
            if (is_sso)
                return sso_buffer;
            else
                return heap._data;
        }

        const char* end() const
        {
            if (is_sso)
                return sso_buffer + size();
            else
                return heap._data + heap.length;
        }

        friend String operator<<(const char* lhs, const String& rhs);

        friend std::ostream& operator<<(std::ostream& os, const String& str)
        {
            os << str.c_str();
            return os;
        }

    private:
        union
        {
            char sso_buffer[bufferSize];
            struct
            {
                char* _data;
                size_t length;
                size_t capacity;
            } heap;
        };
        bool is_sso;
    };

    CommonAPI String operator+(const char* left, String right);

    String operator<<(const char* lhs, const String& rhs);

} // namespace EgLab::Common

#include <functional>

namespace std
{
    template <>
    struct hash<EgLab::Common::String>
    {
        size_t operator()(const EgLab::Common::String& s) const
        {
            const char* str = s.c_str();
            size_t hash = 14695981039346656037ull; // offset basis
            for (size_t i = 0; i < s.size(); i++)
            {
                hash ^= static_cast<size_t>(str[i]);
                hash *= 1099511628211ull; // FNV prime
            }
            return hash;
        }
    };
} // namespace std