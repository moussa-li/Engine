#pragma once

#include <cstddef>
#include <cstdlib>

#include "Common/String.hpp"

namespace EgLab::Platform
{
    // Parses a separator separated float triple ("x,y,z" or "x y z") into a fixed-size array.
    // Missing values are left at 0. Returns the number of parsed floats.
    inline size_t parseFloat3(const Common::String& text, float out[3])
    {
        if (out == nullptr)
        {
            return 0;
        }

        out[0] = 0.0f;
        out[1] = 0.0f;
        out[2] = 0.0f;

        const char* cursor = text.c_str();
        size_t count = 0;
        while (cursor != nullptr && *cursor != '\0' && count < 3)
        {
            while (*cursor == ',' || *cursor == ';' || *cursor == ' ' || *cursor == '\t')
            {
                ++cursor;
            }
            if (*cursor == '\0')
            {
                break;
            }

            char* end = nullptr;
            const float value = std::strtof(cursor, &end);
            if (end == cursor)
            {
                // Skip the remainder of an unexpected token.
                while (*cursor != '\0' && *cursor != ',' && *cursor != ';' && *cursor != ' ' &&
                       *cursor != '\t')
                {
                    ++cursor;
                }
                continue;
            }

            out[count++] = value;
            cursor = end;
        }
        return count;
    }

    // Parses a separator separated float quadruple ("r,g,b,a" or "r g b a") into a fixed-size
    // array. Missing values are left at 0. Returns the number of parsed floats.
    inline size_t parseFloat4(const Common::String& text, float out[4])
    {
        if (out == nullptr)
        {
            return 0;
        }

        out[0] = 0.0f;
        out[1] = 0.0f;
        out[2] = 0.0f;
        out[3] = 0.0f;

        const char* cursor = text.c_str();
        size_t count = 0;
        while (cursor != nullptr && *cursor != '\0' && count < 4)
        {
            while (*cursor == ',' || *cursor == ';' || *cursor == ' ' || *cursor == '\t')
            {
                ++cursor;
            }
            if (*cursor == '\0')
            {
                break;
            }

            char* end = nullptr;
            const float value = std::strtof(cursor, &end);
            if (end == cursor)
            {
                while (*cursor != '\0' && *cursor != ',' && *cursor != ';' && *cursor != ' ' &&
                       *cursor != '\t')
                {
                    ++cursor;
                }
                continue;
            }

            out[count++] = value;
            cursor = end;
        }
        return count;
    }
} // namespace EgLab::Platform
