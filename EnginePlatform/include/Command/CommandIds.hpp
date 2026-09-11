#pragma once

namespace EgLab::Platform
{

    enum class EventId : int32_t
    {
        Unknow = 0,
        UpdateMesh = 200001, // Mesh = 2xxxxx
        MeshImport = 200002,
    };
}