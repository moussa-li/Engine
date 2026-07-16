#pragma once

#include "Common/SharedPtr.hpp"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/Definites.hpp"

namespace EgLab
{
    class Camera;
    class CameraController
    {
    public:
        virtual void update(DeltaTime){};

        CameraController() = default;
        virtual ~CameraController() = default;

        inline void setCamera(SharedPtr<EgLab::Camera> camera)
        {
            _camera = camera;
        }

        inline const SharedPtr<EgLab::Camera> getCamera(SharedPtr<EgLab::Camera>) const
        {
            return _camera;
        }

    protected:
        SharedPtr<EgLab::Camera> _camera;
    };
} // namespace EgLab