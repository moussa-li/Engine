#pragma once

#include <cstdint>
#include <functional>

#include "Common/SharedPtr.hpp"
#include "RenderEngine/Core/Camera.hpp"
#include "RenderEngine/Core/CameraInputBinding.hpp"
#include "RenderEngine/Core/Definites.hpp"

namespace EgLab::RE
{
    class Camera;

    /// Source of the pivot used by the local-orbit gesture, usually the picked mesh center.
    /// Returns false when no mesh has been picked.
    using CameraPivotProvider = std::function<bool(CoordType &)>;

    /// Asks for a pick at a window-space cursor position, used to resolve the local-orbit
    /// pivot when no feature currently needs picking.
    using CameraPivotRequester = std::function<void(float, float)>;

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
         * @param mods Modifier keys held when the button changed state.
         * @return true if the event was handled.
         */
        virtual bool processMouseButton(int button, int action, float xpos, float ypos,
                                        uint32_t mods)
        {
            (void)mods;
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
        /// @param xpos Cursor x-coordinate.
        /// @param ypos Cursor y-coordinate.
        /// @return true if the event was handled.
        virtual bool processMouseScroll(float yoffset, float xpos, float ypos)
        {
            (void)xpos;
            (void)ypos;
            return false;
        };

        /// @brief Sets the mouse chords that trigger each camera gesture.
        virtual void setInputBindings(const CameraInputBindings &bindings)
        {
            (void)bindings;
        }

        /// @brief Sets the window size used to map the cursor to view coordinates.
        virtual void setViewportSize(float width, float height)
        {
            (void)width;
            (void)height;
        }

        /// @brief Sets the pivot source used by the local-orbit gesture.
        virtual void setLocalPivotProvider(CameraPivotProvider provider)
        {
            (void)provider;
        }

        /// @brief Sets the callback that requests a pick for the local-orbit pivot.
        virtual void setLocalPivotRequester(CameraPivotRequester requester)
        {
            (void)requester;
        }

        /// @brief Sets whether zooming anchors on the screen center or on the cursor.
        virtual void setZoomAnchor(CameraZoomAnchor anchor)
        {
            (void)anchor;
        }

        /// @brief Returns the current zoom anchor.
        virtual CameraZoomAnchor getZoomAnchor() const
        {
            return CameraZoomAnchor::ScreenCenter;
        }

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