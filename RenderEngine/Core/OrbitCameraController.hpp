#pragma once

#include "RenderEngine/Core/CameraController.hpp"

namespace EgLab::RE
{
    /// Controls a camera orbiting a target point using mouse input.
    class RenderEngineAPI OrbitCameraController : public CameraController
    {
    public:
        /// @brief Updates the orbit camera state.
        /// @details The elapsed time is supplied by the caller.
        virtual void update(DeltaTime) override;

        /// @brief Creates an orbit camera controller.
        OrbitCameraController() = default;
        /// @brief Releases the orbit camera controller.
        virtual ~OrbitCameraController();

        /// @brief Handles mouse-button input that starts or ends orbit actions.
        /// @param button Mouse button identifier.
        /// @param action Button action, such as press or release.
        /// @param xpos Cursor x-coordinate.
        /// @param ypos Cursor y-coordinate.
        /// @return true if the event was handled.
        bool processMouseButton(int button, int action, float xpos, float ypos) override;

        /// @brief Updates the camera in response to cursor movement.
        /// @param xpos Cursor x-coordinate.
        /// @param ypos Cursor y-coordinate.
        /// @return true if the event was handled.
        bool processMouseMove(float xpos, float ypos) override;

        /// @brief Updates the camera in response to vertical scrolling.
        /// @param yoffset Vertical scroll offset.
        /// @return true if the event was handled.
        bool processMouseScroll(float yoffset) override;

        /// @brief Indicates whether the cursor remains enabled during orbit control.
        /// @return true because this controller keeps the cursor enabled.
        virtual bool enableCursor() const override
        {
            return true;
        }

        /// @brief Sets the camera pan sensitivity.
        /// @details The value is a multiplier for panning.
        void setPanSensitivity(float);

        /// @brief Sets the camera orbit sensitivity.
        /// @details The value is a multiplier for orbit rotation.
        void setOrbitSensitivity(float);

        /// @brief Sets the zoom speed.
        /// @param zoomSpeed Multiplier applied to zoom operations.
        void setZoomSpeed(float zoomSpeed);

    private:
        void startRotate();

        void endRotate();

        void startTraslate();

        void endTraslate();

        void startZoom();

        void endZoom();

        void recalcuteCameraPostition();

    private:
        bool _translate{false};

        bool _rotate{false};

        bool _zoom{true};

        float _lastXPos = -1;
        float _lastYPos = -1;

        int _moveButton = 0; //  current define the left mouse click

        int _rotateButton = 1; //  current define the left mouse click

        CoordType _target = CoordType(0.0f, 0.0f, 0.0f); // look at center
        float _distance = 100.0f;                        // distance of center

        float _panSensitivity = 0.001f;

        float _orbitSensitivity = 0.001f;

        float _zoomSpeed = 1.1f;
    };
} // namespace EgLab::RE