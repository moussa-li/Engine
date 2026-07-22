#pragma once

#include "Common/SharedPtr.hpp"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/Definites.hpp"

namespace EgLab
{
    class Camera;
    class RenderEngineAPI CameraController
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

        /**
         * @brief
         *
         * @param button 0: left 1: right 2: middle
         * @param action 1: click 0: release
         * @param xpos
         * @param ypos
         */
        virtual void processMouseButton(int button, int action, float xpos, float ypos) {};

        virtual void processMouseMove(float xpos, float ypos) {};

        virtual void processMouseScroll(float yoffset) {};

        virtual bool enableCursor() const
        {
            return true;
        }

    protected:
        SharedPtr<EgLab::Camera> _camera;
    };
} // namespace EgLab