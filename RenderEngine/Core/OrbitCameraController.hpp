#pragma once

#include "RenderEngine/Core/CameraController.hpp"

namespace EgLab
{
    class OrbitCameraController : public CameraController
    {
    public:
        virtual void update(DeltaTime) override;

        OrbitCameraController() = default;
        virtual ~OrbitCameraController() = default;
    };
} // namespace EgLab