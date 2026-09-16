#pragma once

#include "Common/BBox.hpp"
#include "Common/Matrix.hpp"
#include "Definites.hpp"
#include "RenderEngine/Core/Quatf.hpp"
#include "RenderEngineAPI.hpp"

namespace EgLab::RE
{
    constexpr float YAW = -90.0f;
    constexpr float PITCH = 0.0f;
    constexpr float SPEED = 2.5f;
    constexpr float SENSITIVITY = 0.1f;
    constexpr float ZOOM = 10.0f;

    class RenderEngineAPI Camera
    {
    public:
        Camera(const unsigned int &width, const unsigned int &height,
               CoordType position = CoordType(0.0f, 0.0f, 1000.0f),
               CoordType up = CoordType(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

        Common::Matrix4f perspective() const;

        Common::Matrix4f perspective(const Common::BBox<Scalar, 3> &bounds) const;

        void fitView(const Common::BBox<Scalar, 3> &bounds, float margin = 1.1f);

        Common::Matrix4f view() const;

        void lookAt(const CoordType &position, const CoordType &front, const CoordType &up);

        void setWH(unsigned int width, unsigned int height);

        // --- Getters ---
        CoordType getPosition() const
        {
            return _position;
        }
        CoordType getFront() const
        {
            return _front;
        }
        CoordType getUp() const
        {
            return _up;
        }
        CoordType getRight() const
        {
            return _right;
        }

        CoordType getTarget() const
        {
            return _target;
        }

        Quatf getOrientation() const
        {
            return _orientation;
        }

        float getYaw() const
        {
            return _yaw;
        }
        float getPitch() const
        {
            return _pitch;
        }
        float getZoom() const
        {
            return _zoom;
        }

        // --- Setters ---
        void setPosition(const CoordType &pos)
        {
            _position = pos;
        }

        void setTarget(const CoordType &target)
        {
            _target = target;
        }

        void setFront(const CoordType &front);

        void setRigth(const CoordType &right)
        {
            _right = right;
        }

        void setUp(const CoordType &up)
        {
            _up = up;
        }

        void setYaw(float yaw)
        {
            _yaw = yaw;
        }
        void setPitch(float pitch)
        {
            _pitch = pitch;
            // if (_pitch > 89.0f) _pitch = 89.0f;
            // if (_pitch < -89.0f) _pitch = -89.0f;
        }
        void setZoom(float zoom)
        {
            _zoom = zoom;
        }

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
        float _zoom;

        unsigned int _width;
        unsigned int _height;
    };

} // namespace EgLab::RE