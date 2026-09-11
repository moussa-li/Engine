#pragma once

#include <stddef.h>

#include <cstdint>
#include <type_traits>

#include "Common/Protocol.hpp"


namespace EgLab::Platform
{
    struct ImportMeshParam
    {
        char fileDir[128];
    };

    struct UpdateMeshParam
    {
        uint32_t dataSize;
        uint8_t* data;
    };

    static_assert(std::is_trivially_copyable<ImportMeshParam>::value,
                  "ImportMeshParam must be trivially copyable");
    static_assert(std::is_trivially_copyable<UpdateMeshParam>::value,
                  "UpdateMeshParam must be trivially copyable");

} // namespace EgLab::Platform