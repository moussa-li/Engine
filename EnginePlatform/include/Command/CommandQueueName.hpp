#pragma once

#include <cstdint>
#include <string>

#include "Common/String.hpp"

namespace EgLab::Platform
{
    inline Common::String commandMemoryName(uint32_t processId)
    {
        const std::string name = "Command Memory-" + std::to_string(processId);
        return Common::String(name.c_str());
    }
} // namespace EgLab::Platform