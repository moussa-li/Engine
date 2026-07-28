#include "RenderEngine/Core/OrbitCameraController.hpp"

#include "Common/Log.hpp"

namespace EgLab
{
    void OrbitCameraController::update(DeltaTime)
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

            pos = pos - right * (xoffset * _panSensitivity);
            pos = pos + up * (yoffset * _panSensitivity);
            _camera->setPosition(pos);
        }

        recalcuteCameraPostition();

        _lastXPos = xpos;
        _lastYPos = ypos;

        // LOG(INFO) << "xpos : " << xpos;
        // LOG(INFO) << "ypos : " << ypos;
    }

    void OrbitCameraController::processMouseScroll(float yoffset)
    {
        // LOG(INFO) << "yoffset : " << yoffset;
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

        // _camera->setPosition(pos);
        _camera->lookAt(pos, _target, _camera->getUp());
    }

} // namespace EgLab
