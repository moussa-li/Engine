#pragma once
/**
 * @file Return.hpp
 * @author Moussa-Li
 * @brief the return status
 * @date 2025-10-26
 */

namespace EgLab::Common
{
    enum class Return
    {
        Succeed,
        Failed,
        NotImplacementYet,
        FileNotFound,
        BadInput
    };
}