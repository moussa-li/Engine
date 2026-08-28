#include "RenderEngine/Core/OrbitCameraController.hpp"

#include "Common/Log.hpp"

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
            auto zoom = _camera->getZoom();

            pos = pos - right * (xoffset * _panSensitivity) * zoom;
            pos = pos + up * (yoffset * _panSensitivity) * zoom;

            // front = front - right * (xoffset * _panSensitivity);
            // front = front + up * (yoffset * _panSensitivity);

            _camera->setPosition(pos);
            // _camera->setFront(front);
        }

        if (_rotate)
        {
            auto yaw = _camera->getYaw();
            auto pitch = _camera->getPitch();
            xoffset *= _orbitSensitivity;
            yoffset *= _orbitSensitivity;

            yaw += xoffset;
            pitch += yoffset;

            // 限制 Pitch 角度，防止相机翻转（万向节死锁）
            if (pitch > 89.0f) pitch = 89.0f;
            if (pitch < -89.0f) pitch = -89.0f;

            _camera->setPitch(pitch);
            _camera->setYaw(yaw);

            // 根据 Yaw 和 Pitch 重新计算相机的朝向向量 (Front)
            CoordType front;
            front.x() = std::cos(radians(yaw)) * std::cos(radians(pitch));
            front.y() = std::sin(radians(pitch));
            front.z() = std::sin(radians(yaw)) * std::cos(radians(pitch));
            front.normalize();

            _camera->setFront(front);

            // 核心：重新计算相机位置，使其始终保持在目标点后方距离为 zoom 的地方
            // 假设你的相机有 getTarget() 方法，如果没有，可以用 pos + front * zoom 来推导
            // auto target = _camera->getPosition() + _camera->getFront() * _camera->getZoom();
            CoordType target(0, 0, 0);
            auto zoom = _camera->getZoom();

            CoordType newPos = target - front * zoom;
            _camera->setPosition(newPos);
        }

        recalcuteCameraPostition();

        _lastXPos = xpos;
        _lastYPos = ypos;
    }

    void OrbitCameraController::processMouseScroll(float yoffset)
    {
        auto zoom = _camera->getZoom();

        if (yoffset > 0)
        {
            zoom /= _zoomSpeed;
        }
        else if (yoffset < 0)
        {
            zoom *= _zoomSpeed;
        }

        zoom = std::max(0.1f, std::min(zoom, 1000.0f));
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
