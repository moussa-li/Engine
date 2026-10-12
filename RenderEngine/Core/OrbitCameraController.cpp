#include "RenderEngine/Core/OrbitCameraController.hpp"

#include "Common/Log.hpp"
#include "RenderEngine/Core/Quatf.hpp"

namespace EgLab::RE
{
    /**
     * @brief from degress get the radians
     *
     * @param degress
     * @return float
     */
    inline float radians(const float &degress)
    {
        return degress * static_cast<float>(0.01745329251994329576923690768489);
    }

    void OrbitCameraController::update(DeltaTime)
    {
    }

    OrbitCameraController::~OrbitCameraController()
    {
    }

    bool OrbitCameraController::processMouseButton(int button, int action, float xpos, float ypos)
    {
        if (_camera == nullptr) return false;
        if (button == _moveButton)
        {
            if (action)
            {
                _translate = true;
                _lastXPos = xpos;
                _lastYPos = ypos;
            }
            else
            {
                _translate = false;
            }
        }
        if (button == _rotateButton)
        {
            if (action)
            {
                _rotate = true;
                _lastXPos = xpos;
                _lastYPos = ypos;
            }
            else
            {
                _rotate = false;
            }
        }
        return false;
    }

    bool OrbitCameraController::processMouseMove(float xpos, float ypos)
    {
        if (_camera == nullptr) return false;

        float xoffset = xpos - _lastXPos;
        float yoffset = ypos - _lastYPos;

        bool isChanged = false;

        if (_translate)
        {
            CoordType right = _camera->getRight();
            CoordType up = _camera->getUp();
            CoordType pos = _camera->getPosition();
            CoordType front = _camera->getFront();
            CoordType target = _camera->getTarget();
            auto zoom = _camera->getZoom();

            const float panSensitivity = _panSensitivity * _camera->getMoveSensitivity();
            pos = pos - right * (xoffset * panSensitivity) * zoom;
            pos = pos + up * (yoffset * panSensitivity) * zoom;
            target = target - right * (xoffset * panSensitivity) * zoom;
            target = target + up * (yoffset * panSensitivity) * zoom;
            _camera->setPosition(pos);
            _camera->setTarget(target);

            isChanged = true;
        }

        if (_rotate)
        {
            auto yaw = _camera->getYaw();
            auto pitch = _camera->getPitch();
            const float orbitSensitivity = _orbitSensitivity * _camera->getRotateSensitivity();
            xoffset *= orbitSensitivity;
            yoffset *= orbitSensitivity;

            auto up = _camera->getUp();
            auto right = _camera->getRight();

            auto orientation = _camera->getOrientation();
            CoordType localUp = orientation.rotateVector(CoordType(0, 1, 0));
            Quatf yawQuat = Quatf::fromAxisAndAngle(localUp, -xoffset);

            Quatf pitchQuat = Quatf::fromAxisAndAngle(CoordType(1, 0, 0), -yoffset);

            orientation = yawQuat * orientation * pitchQuat;
            orientation.normalize(); // 必须归一化！
            _camera->setOrientation(orientation);

            CoordType newFront = orientation.rotateVector(CoordType(0, 0, -1));
            newFront.normalize();
            _camera->setFront(newFront);

            _camera->setRigth(orientation.rotateVector(CoordType(1, 0, 0)));
            _camera->setUp(orientation.rotateVector(CoordType(0, 1, 0)));

            CoordType target = _camera->getTarget();
            CoordType position = _camera->getPosition();
            float distance = (target - position).length();
            position = target - newFront * distance;
            _camera->setPosition(position);
            isChanged = true;
        }

        recalcuteCameraPostition();

        _lastXPos = xpos;
        _lastYPos = ypos;
        return isChanged;
    }

    bool OrbitCameraController::processMouseScroll(float yoffset)
    {
        if (_camera == nullptr) return false;

        const float zoomSpeed = 1.0f + (_zoomSpeed - 1.0f) * _camera->getZoomSensitivity();
        auto target = _camera->getTarget();
        auto position = _camera->getPosition();
        auto distance = (target - position).length();
        auto newPos = position + _camera->getFront() * (yoffset * zoomSpeed);
        _camera->setPosition(newPos);

        auto zoom = _camera->getZoom();
        if (yoffset > 0)
        {
            zoom /= zoomSpeed;
        }
        else if (yoffset < 0)
        {
            zoom *= zoomSpeed;
        }

        // zoom = std::max(0.1f, std::min(zoom, 1000.0f));
        _camera->setZoom(zoom);
        return true;
    }

    void OrbitCameraController::setPanSensitivity(float sensitivity)
    {
        _panSensitivity = sensitivity;
    }

    void OrbitCameraController::setOrbitSensitivity(float sensitivity)
    {
        _orbitSensitivity = sensitivity;
    }

    void OrbitCameraController::setZoomSpeed(float zoomSpeed)
    {
        _zoomSpeed = zoomSpeed;
    }

    void OrbitCameraController::recalcuteCameraPostition()
    {
        return;
        if (_camera == nullptr) return;
        float yawRad = _camera->getYaw() * 3.14159265f / 180.0f;
        float pitchRad = _camera->getPitch() * 3.14159265f / 180.0f;

        CoordType pos;
        pos.x() = _target.x() + _distance * cos(pitchRad) * cos(yawRad);
        pos.y() = _target.y() + _distance * sin(pitchRad);
        pos.z() = _target.z() + _distance * cos(pitchRad) * sin(yawRad);

        _camera->lookAt(pos, _target, _camera->getUp());
    }

} // namespace EgLab::RE
