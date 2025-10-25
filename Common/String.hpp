#pragma once

/**
 * @file String.h
 * @author Moussa-Li
 * @brief a quickly and lower memory string
 * @date 2025-09-28
 */


#include "Common/CommonAPI.hpp"

#include <iostream>

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

        size_t size() const;

        const char* c_str() const;

        void append(const char* str);

        String operator = (const String& other);

        String operator +(const String& other)const;

        String &operator +=(const String& other);



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