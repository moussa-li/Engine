#pragma once

/**
 * @file String.h
 * @author Moussa-Li
 * @brief a quickly and lower memory string
 * @date 2025-09-28
 */


#include "Common/CommonAPI.hpp"
#include <cstddef>


namespace EgLab 
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

        size_t size() const;

        const char* c_str() const;

        void append(const char* str);

        String& operator = (const String& other);

        String operator +(const String& other)const;

        String &operator +=(const String& other);

        bool operator==(const String& other) const;


    private:
    union {
        char sso_buffer[bufferSize];
        struct {
            char* _data;
            size_t length;
            size_t capacity;
        } heap;

        };
        bool is_sso;
    };

    String operator+(const char* left, String right);

}

#include <functional>

namespace std {
    template<>
    struct hash<EgLab::String> {
        size_t operator()(const EgLab::String& s) const {
            const char* str = s.c_str();
            size_t hash = 14695981039346656037ull; // offset basis
            for(size_t i = 0; i < s.size(); i++) {
                hash ^= static_cast<size_t>(str[i]);
                hash *= 1099511628211ull; // FNV prime
            }
            return hash;
        }
    };
}