#pragma once
#include <filesystem>

#include "Common/String.hpp"

namespace EgLab
{

    inline Common::String getTestDataDir(const Common::String &file)
    {
        std::filesystem::path root(__FILE__);
        root = root.parent_path();
        root /= "testdata";
        root /= file.c_str();
        return Common::String(root.generic_string().c_str());
    }
} // namespace EgLab