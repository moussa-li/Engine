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

    void OrbitCameraController::processMouseButton(int button, int action, float xpos, float ypos)
    {
        if (_camera == nullptr) return;
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
    }

    void OrbitCameraController::processMouseMove(float xpos, float ypos)
    {
        if (_camera == nullptr) return;

        float xoffset = xpos - _lastXPos;
        float yoffset = ypos - _lastYPos;

        if (_translate)
        {
            CoordType right = _camera->getRight();
            CoordType up = _camera->getUp();
            CoordType pos = _camera->getPosition();
            CoordType front = _camera->getFront();
            CoordType target = _camera->getTarget();
            auto zoom = _camera->getZoom();

            pos = pos - right * (xoffset * _panSensitivity) * zoom;
            pos = pos + up * (yoffset * _panSensitivity) * zoom;
            target = target - right * (xoffset * _panSensitivity) * zoom;
            target = target + up * (yoffset * _panSensitivity) * zoom;
            _camera->setPosition(pos);
            _camera->setTarget(target);
        }

        if (_rotate)
        {
            auto yaw = _camera->getYaw();
            auto pitch = _camera->getPitch();
            xoffset *= _orbitSensitivity;
            yoffset *= _orbitSensitivity;

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
        }

        recalcuteCameraPostition();

        _lastXPos = xpos;
        _lastYPos = ypos;
    }

    void OrbitCameraController::processMouseScroll(float yoffset)
    {
        auto target = _camera->getTarget();
        auto position = _camera->getPosition();
        auto distance = (target - position).length();
        auto newPos = position + _camera->getFront() * (yoffset * _zoomSpeed * distance * 0.1f);
        _camera->setPosition(newPos);

        auto zoom = _camera->getZoom();
        if (yoffset > 0)
        {
            zoom /= _zoomSpeed;
        }
        else if (yoffset < 0)
        {
            zoom *= _zoomSpeed;
        }

        // zoom = std::max(0.1f, std::min(zoom, 1000.0f));
        _camera->setZoom(zoom);
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
