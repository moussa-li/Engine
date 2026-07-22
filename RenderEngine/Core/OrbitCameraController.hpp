#pragma once

#include "RenderEngine/Core/CameraController.hpp"

namespace EgLab
{
    class RenderEngineAPI OrbitCameraController : public CameraController
    {
    public:
        virtual void update(DeltaTime) override;

        OrbitCameraController() = default;
        virtual ~OrbitCameraController() = default;

        void processMouseButton(int button, int action, float xpos, float ypos) override;

        void processMouseMove(float xpos, float ypos) override;

        void processMouseScroll(float yoffset) override;

        virtual bool enableCursor() const override
        {
            return true;
        }

    private:
        void startRotate();

        void endRotate();

        void startTraslate();

        void endTraslate();

        void startZoom();

        void endZoom();

        void recalcuteCameraPostition();

    private:
        bool _rotate{false};

        bool _translate{false};

        bool _zoom{true};

        float _lastXPos = -1;
        float _lastYPos = -1;

        int _moveButton = 0; //  current define the left mouse click

        CoordType _target = CoordType(0.0f, 0.0f, 0.0f); // look at center
        float _distance = 100.0f;                        // distance of center

        float _panSensitivity = 1.0f;
    };
} // namespace EgLab