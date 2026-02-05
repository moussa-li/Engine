#include "Common/String.hpp"

#include <string.h>

#include <iostream>

#include "Common/DynamicArray.hpp"
#include "Common/Log.hpp"

namespace EgLab
{
    constexpr size_t expansionCoefficient = 2;

    String::String() : is_sso(true), sso_buffer("\0")
    {
    }

    String::String(const char* str) : is_sso(true), sso_buffer("\0")
    {
        append(str);
    }

    String::String(const String& other)
    {
        is_sso = other.is_sso;
        if (other.is_sso)
        {
            strcpy(sso_buffer, other.sso_buffer);
        }
        else
        {
            heap.capacity = other.heap.capacity;
            heap.length = other.heap.length;
            heap._data = new char[other.heap.capacity];
            memcpy(heap._data, other.heap._data, other.heap.length);
        }
    }

    String::String(String&& other)
    {
        is_sso = other.is_sso;
        if (other.is_sso)
        {
            strcpy(sso_buffer, other.sso_buffer);
        }
        else
        {
            heap = other.heap;
        }
    }

    String::String(const unsigned char str[], const unsigned long size) : is_sso(true)
    {
        size_t len = size / sizeof(str[0]) + 1;
        if (is_sso && len < sizeof(sso_buffer))
        {
            memcpy(sso_buffer, str, len);
            sso_buffer[len] = '\0';
        }
        else
        {
            heap._data = new char[len + 1];
            memcpy(heap._data, str, len);
            heap._data[len] = '\0';
            heap.length = len;
            heap.capacity = len * expansionCoefficient;
            is_sso = false;
        }
    }

    String::String(const char* str, const unsigned long size) : is_sso(true)
    {
        size_t len = size / sizeof(str[0]);
        if (is_sso && len < sizeof(sso_buffer))
        {
            memcpy(sso_buffer, str, len);
            sso_buffer[len] = '\0';
        }
        else
        {
            heap._data = new char[len + 1];
            memcpy(heap._data, str, len);
            heap._data[len] = '\0';
            heap.length = len;
            heap.capacity = len * expansionCoefficient;
            is_sso = false;
        }
    }

    String& String::operator=(const String& other)
    {
        is_sso = other.is_sso;
        if (other.is_sso)
        {
            strcpy(sso_buffer, other.sso_buffer);
        }
        else
        {
            heap.capacity = other.heap.capacity;
            heap.length = other.heap.length;
            heap._data = new char[other.heap.capacity];
            memcpy(heap._data, other.heap._data, other.heap.length);
        }
        return *this;
    }

    bool String::operator==(const char* str) const
    {
        size_t len = strlen(str);
        String other(str, len);
        return *this == other;
    }

    bool String::operator==(const String& other) const
    {
        if (this->size() != other.size()) return false;
        const char* str = this->c_str();
        const char* otherStr = other.c_str();
        for (size_t i = 0; i < this->size(); ++i)
        {
            if (str[i] != otherStr[i]) return false;
        }
        return true;
    }

    String& String::operator+=(const String& other)
    {
        append(other.c_str());
        return *this;
    }

    size_t String::size() const
    {
        return is_sso ? strlen(sso_buffer) : heap.length;
    }

    const char* String::c_str() const
    {
        return is_sso ? sso_buffer : heap._data;
    }

    void String::append(const char* str)
    {
        size_t new_len = size() + strlen(str);
        if (is_sso && new_len < sizeof(sso_buffer))
        {
            strcat(sso_buffer, str);
        }
        else
        {
            if (is_sso)
            {
                char tmp_buffer[bufferSize];
                strcpy(tmp_buffer, sso_buffer);
                heap.capacity = new_len * expansionCoefficient;
                heap._data = new char[heap.capacity];
                strcpy(heap._data, tmp_buffer);
                is_sso = false;
            }
            else if (new_len > heap.capacity)
            {
                size_t new_capacity = new_len * expansionCoefficient;
                char* new_data = new char[new_capacity];
                strcpy(new_data, heap._data);
                delete[] heap._data;
                heap._data = new_data;
                heap.capacity = new_capacity;
            }
            strcat(heap._data, str);
            heap.length = new_len;
        }
    }

    String String::operator+(const String& other) const
    {
        String res;
        res.append(this->c_str());
        res.append(other.c_str());
        return res;
    }

    String operator+(const char* left, String right)
    {
        String res(left);
        return res + right;
    }

} // namespace EgLab