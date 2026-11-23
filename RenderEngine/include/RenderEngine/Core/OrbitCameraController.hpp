#pragma once

#include <atomic>

#include "RenderEngine/Core/CameraController.hpp"
#include "RenderEngine/Core/CameraInputBinding.hpp"

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
        /// @param mods Modifier keys held when the button changed state.
        /// @return true if the event was handled.
        bool processMouseButton(int button, int action, float xpos, float ypos,
                                uint32_t mods) override;

        /// @brief Updates the camera in response to cursor movement.
        /// @param xpos Cursor x-coordinate.
        /// @param ypos Cursor y-coordinate.
        /// @return true if the event was handled.
        bool processMouseMove(float xpos, float ypos) override;

        /// @brief Updates the camera in response to vertical scrolling.
        /// @param yoffset Vertical scroll offset.
        /// @param xpos Cursor x-coordinate.
        /// @param ypos Cursor y-coordinate.
        /// @return true if the event was handled.
        bool processMouseScroll(float yoffset, float xpos, float ypos) override;

        /// @brief Sets the mouse chords that trigger each camera gesture.
        void setInputBindings(const CameraInputBindings &bindings) override;

        /// @brief Sets the window size used to map the cursor to view coordinates.
        void setViewportSize(float width, float height) override;

        /// @brief Sets the pivot source used by the rotate gestures.
        void setLocalPivotProvider(CameraPivotProvider provider) override;

        /// @brief Sets whether zooming anchors on the screen center or on the cursor.
        void setZoomAnchor(CameraZoomAnchor anchor) override;

        /// @brief Returns the current zoom anchor.
        CameraZoomAnchor getZoomAnchor() const override;

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
        bool pan(float xoffset, float yoffset);

        bool rotate(float xoffset, float yoffset, const CoordType &pivot, bool localAxis);

        bool applyZoom(float steps, float cursorX, float cursorY);

        void recalcuteCameraPostition();

    private:
        CameraInputBindings _bindings{CameraInputBindings::defaults()};

        CameraAction _activeAction{CameraAction::None};
        int _activeButton{static_cast<int>(CameraMouseButton::None)};

        CoordType _activePivot;
        bool _hasActivePivot{false};

        CameraPivotProvider _pivotProvider;

        std::atomic<CameraZoomAnchor> _zoomAnchor{CameraZoomAnchor::ScreenCenter};
        float _viewportWidth{0.0f};
        float _viewportHeight{0.0f};

        float _lastXPos = -1.0f;
        float _lastYPos = -1.0f;

        CoordType _target = CoordType(0.0f, 0.0f, 0.0f); // look at center
        float _distance = 100.0f;                        // distance of center

        float _panSensitivity = 0.001f;

        float _orbitSensitivity = 0.001f;

        float _zoomSpeed = 1.1f;
    };
} // namespace EgLab::RE