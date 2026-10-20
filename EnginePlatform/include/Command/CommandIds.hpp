#pragma once

#include <cstdint>

namespace EgLab::Platform
{

    enum class EventId : int32_t
    {
        Unknow = 0,
        ShowPanel = 100001,
        FitView = 100002,
        UpdateMesh = 200001, // Mesh = 2xxxxx
        MeshImport = 200002,
        MeshDelete = 200003,
        MeshPickInfo = 200004,
    };
}