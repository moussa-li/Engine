#pragma once

#include <cstddef>
#include <cstdint>

#include "Common/String.hpp"

namespace EgLab::Platform
{
    // Parses a separator separated id list ("1, 2, 3" or "1 2 3") into a fixed-size array.
    // Returns the number of parsed ids, clamped to capacity.
    template <typename T>
    size_t parseIdList(const Common::String& text, T* values, size_t capacity)
    {
        if (values == nullptr || capacity == 0)
        {
            return 0;
        }

        size_t count = 0;
        const char* cursor = text.c_str();
        while (cursor != nullptr && *cursor != '\0' && count < capacity)
        {
            while (*cursor == ',' || *cursor == ';' || *cursor == ' ' || *cursor == '\t')
            {
                ++cursor;
            }
            if (*cursor == '\0')
            {
                break;
            }

            uint64_t value = 0;
            bool hasDigits = false;
            while (*cursor >= '0' && *cursor <= '9')
            {
                value = value * 10 + static_cast<uint64_t>(*cursor - '0');
                hasDigits = true;
                ++cursor;
            }

            if (hasDigits)
            {
                values[count++] = static_cast<T>(value);
            }

            // Skip the remainder of an unexpected token.
            while (*cursor != '\0' && *cursor != ',' && *cursor != ';')
            {
                ++cursor;
            }
        }
        return count;
    }

    // Formats an id array as "1, 2, 3" for EventPacket string fields and the UI.
    template <typename T>
    Common::String formatIdList(const T* values, size_t count)
    {
        Common::String result;
        if (values == nullptr)
        {
            return result;
        }

        char digits[24];
        for (size_t index = 0; index < count; ++index)
        {
            if (index != 0)
            {
                result += Common::String(", ");
            }

            uint64_t value = static_cast<uint64_t>(values[index]);
            size_t position = sizeof(digits) - 1;
            digits[position] = '\0';
            do
            {
                digits[--position] = static_cast<char>('0' + value % 10);
                value /= 10;
            } while (value != 0);

            result += Common::String(digits + position);
        }
        return result;
    }
} // namespace EgLab::Platform
