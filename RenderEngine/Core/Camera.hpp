#pragma once

#include "Common/BBox.hpp"
#include "Common/Matrix.hpp"
#include "Definites.hpp"
#include "RenderEngine/Core/Quatf.hpp"
#include "RenderEngineAPI.hpp"

namespace EgLab::RE
{
    /// Default yaw angle in degrees.
    constexpr float YAW = -90.0f;
    /// Default pitch angle in degrees.
    constexpr float PITCH = 0.0f;
    /// Default camera movement speed.
    constexpr float SPEED = 2.5f;
    /// Default mouse input sensitivity.
    constexpr float SENSITIVITY = 0.1f;
    /// Default camera zoom value.
    constexpr float ZOOM = 10.0f;

    /**
     * @brief Represents a perspective camera used by the renderer.
     * @details Stores the camera pose, projection parameters, viewport size, and input
     * sensitivities.
     */
    class RenderEngineAPI Camera
    {
    public:
        /**
         * @brief Constructs a camera for the specified viewport.
         * @param width Viewport width in pixels.
         * @param height Viewport height in pixels.
         * @param position Initial camera position.
         * @param up Initial world-up direction.
         * @param yaw Initial yaw angle in degrees.
         * @param pitch Initial pitch angle in degrees.
         */
        Camera(const unsigned int &width, const unsigned int &height,
               CoordType position = CoordType(0.0f, 0.0f, 1000.0f),
               CoordType up = CoordType(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

        /// @brief Builds the perspective projection matrix for the current viewport.
        Common::Matrix4f perspective() const;

        /**
         * @brief Builds a perspective projection matrix fitted to the given bounds.
         * @param bounds World-space bounds to fit in the view.
         * @return The perspective projection matrix.
         */
        Common::Matrix4f perspective(const Common::BBox<Scalar, 3> &bounds) const;

        /**
         * @brief Adjusts the camera to frame the given bounds.
         * @param bounds World-space bounds to frame.
         * @param margin Scale factor applied around the bounds.
         */
        void fitView(const Common::BBox<Scalar, 3> &bounds, float margin = 1.1f);

        /// @brief Builds the view matrix from the current camera pose.
        Common::Matrix4f view() const;

        /**
         * @brief Sets the camera position and orientation vectors.
         * @param position Camera position.
         * @param front Forward direction.
         * @param up Up direction.
         */
        void lookAt(const CoordType &position, const CoordType &front, const CoordType &up);

        /**
         * @brief Updates the viewport dimensions used for projection.
         * @param width Viewport width in pixels.
         * @param height Viewport height in pixels.
         */
        void setWH(unsigned int width, unsigned int height);

        // --- Getters ---
        /// @brief Returns the camera position.
        CoordType getPosition() const
        {
            return _position;
        }
        /// @brief Returns the camera forward direction.
        CoordType getFront() const
        {
            return _front;
        }
        /// @brief Returns the camera up direction.
        CoordType getUp() const
        {
            return _up;
        }
        /// @brief Returns the camera right direction.
        CoordType getRight() const
        {
            return _right;
        }

        /// @brief Returns the camera target point.
        CoordType getTarget() const
        {
            return _target;
        }

        /// @brief Returns the camera orientation quaternion.
        Quatf getOrientation() const
        {
            return _orientation;
        }

        /// @brief Returns the yaw angle in degrees.
        float getYaw() const
        {
            return _yaw;
        }
        /// @brief Returns the pitch angle in degrees.
        float getPitch() const
        {
            return _pitch;
        }
        /// @brief Returns the current zoom value.
        float getZoom() const
        {
            return _zoom;
        }
        /// @brief Returns the movement sensitivity multiplier.
        float getMoveSensitivity() const
        {
            return _moveSensitivity;
        }
        /// @brief Returns the rotation sensitivity multiplier.
        float getRotateSensitivity() const
        {
            return _rotateSensitivity;
        }
        /// @brief Returns the zoom sensitivity multiplier.
        float getZoomSensitivity() const
        {
            return _zoomSensitivity;
        }

        // --- Setters ---
        /** @brief Sets the camera position. @param pos New camera position. */
        void setPosition(const CoordType &pos)
        {
            _position = pos;
        }

        /** @brief Sets the camera target point. @param target New target point. */
        void setTarget(const CoordType &target)
        {
            _target = target;
        }

        /** @brief Sets the camera forward direction. @param front New forward direction. */
        void setFront(const CoordType &front);

        /** @brief Sets the camera right direction. @param right New right direction. */
        void setRigth(const CoordType &right)
        {
            _right = right;
        }

        /** @brief Sets the camera up direction. @param up New up direction. */
        void setUp(const CoordType &up)
        {
            _up = up;
        }

        /** @brief Sets the yaw angle in degrees. @param yaw New yaw angle. */
        void setYaw(float yaw)
        {
            _yaw = yaw;
        }
        /** @brief Sets the pitch angle in degrees. @param pitch New pitch angle. */
        void setPitch(float pitch)
        {
            _pitch = pitch;
            // if (_pitch > 89.0f) _pitch = 89.0f;
            // if (_pitch < -89.0f) _pitch = -89.0f;
        }
        /** @brief Sets the zoom value. @param zoom New zoom value. */
        void setZoom(float zoom)
        {
            _zoom = zoom;
        }
        /** @brief Sets the movement sensitivity multiplier. @param sensitivity New multiplier. */
        void setMoveSensitivity(float sensitivity)
        {
            _moveSensitivity = sensitivity;
        }
        /** @brief Sets the rotation sensitivity multiplier. @param sensitivity New multiplier. */
        void setRotateSensitivity(float sensitivity)
        {
            _rotateSensitivity = sensitivity;
        }
        /** @brief Sets the zoom sensitivity multiplier. @param sensitivity New multiplier. */
        void setZoomSensitivity(float sensitivity)
        {
            _zoomSensitivity = sensitivity;
        }

        /** @brief Sets the camera orientation. @param orientation New orientation quaternion. */
        void setOrientation(Quatf orientation)
        {
            _orientation = orientation;
        }

    private:
        void updateCameraVectors();

    private:
        CoordType _position;
        CoordType _front;
        CoordType _up;
        CoordType _right;
        CoordType _worldUp;

        CoordType _target;

        Quatf _orientation;

        float _yaw;
        float _pitch;

        float _lastX;
        float _lastY;
        bool _firstMouse = true;

        float _movementSpeed;
        float _mouseSensitivity;
        float _moveSensitivity = 1.0f;
        float _rotateSensitivity = 1.0f;
        float _zoomSensitivity = 1.0f;
        float _zoom;

        unsigned int _width;
        unsigned int _height;
    };

} // namespace EgLab::RE