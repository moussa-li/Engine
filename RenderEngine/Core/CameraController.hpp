#pragma once

#include "Common/SharedPtr.hpp"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/Definites.hpp"

namespace EgLab::RE
{
    class Camera;
    /// Base class for camera input and update controllers.
    class RenderEngineAPI CameraController
    {
    public:
        /// @brief Updates the controlled camera for the elapsed time.
        /// @details The elapsed time is supplied by the caller.
        virtual void update(DeltaTime){};

        /// @brief Constructs a camera controller without an attached camera.
        CameraController();
        /// @brief Destroys the camera controller.
        virtual ~CameraController();

        /// @brief Sets the camera controlled by this controller.
        /// @param camera Camera to control.
        inline void setCamera(Common::SharedPtr<EgLab::RE::Camera> camera)
        {
            _camera = camera;
        }

        /// @brief Returns the currently controlled camera.
        /// @return The currently controlled camera.
        inline const Common::SharedPtr<EgLab::RE::Camera> getCamera(
            Common::SharedPtr<EgLab::RE::Camera>) const
        {
            return _camera;
        }

        /**
         * @brief Handles a mouse-button event.
         *
         * @param button 0: left 1: right 2: middle
         * @param action 1: click 0: release
         * @param xpos Cursor x-coordinate.
         * @param ypos Cursor y-coordinate.
         * @return true if the event was handled.
         */
        virtual bool processMouseButton(int button, int action, float xpos, float ypos)
        {
            return false;
        };

        /// @brief Handles cursor movement.
        /// @param xpos Cursor x-coordinate.
        /// @param ypos Cursor y-coordinate.
        /// @return true if the event was handled.
        virtual bool processMouseMove(float xpos, float ypos)
        {
            return false;
        };

        /// @brief Handles a mouse-wheel event.
        /// @param yoffset Vertical scroll offset.
        /// @return true if the event was handled.
        virtual bool processMouseScroll(float yoffset)
        {
            return false;
        };

        /// @brief Indicates whether the cursor should remain enabled.
        /// @return true when the cursor is enabled.
        virtual bool enableCursor() const
        {
            return true;
        }

    protected:
        Common::SharedPtr<EgLab::RE::Camera> _camera{nullptr};
    };
} // namespace EgLab::RE