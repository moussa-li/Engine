#include "RenderEngine/Core/OrbitCameraController.hpp"

#include <cmath>

#include "Common/Log.hpp"
#include "Common/Utils.hpp"
#include "RenderEngine/Core/Quatf.hpp"

namespace EgLab::RE
{
    namespace
    {
        // Vertical drag distance, in pixels, that equals one wheel step.
        constexpr float kZoomDragStepsPerPixel = 0.01f;
        constexpr float kMinZoom = 1e-4f;
        // How long a local orbit waits for its pivot pick before falling back to the target.
        constexpr float kLocalPivotTimeoutSeconds = 0.25f;
    } // namespace

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

    void OrbitCameraController::update(DeltaTime deltaTime)
    {
        if (_camera)
        {
            _camera->updateCameraTransition(deltaTime);
        }

        if (!_pendingLocalPivot)
        {
            return;
        }

        // The pivot pick runs on the render thread; wait a couple of frames for its result
        // before giving up and rotating around the view target instead.
        _pendingLocalPivotElapsed += deltaTime;
        CoordType pivot;
        if (_pivotProvider && _pivotProvider(pivot))
        {
            _activePivot = pivot;
            _hasActivePivot = true;
            _pendingLocalPivot = false;
            return;
        }

        if (_pendingLocalPivotElapsed > kLocalPivotTimeoutSeconds)
        {
            _pendingLocalPivot = false;
        }
    }

    OrbitCameraController::~OrbitCameraController()
    {
    }

    bool OrbitCameraController::processMouseButton(int button, int action, float xpos, float ypos,
                                                   uint32_t mods)
    {
        if (_camera == nullptr) return false;

        if (action != 0)
        {
            if (_activeAction != CameraAction::None) return false;

            const CameraAction requested = _bindings.resolve(button, mods);
            if (requested == CameraAction::None) return false;

            _activeAction = requested;
            _activeButton = button;
            _lastXPos = xpos;
            _lastYPos = ypos;
            _hasActivePivot = false;
            _pendingLocalPivot = false;
            _pendingLocalPivotElapsed = 0.0f;

            if (requested == CameraAction::RotateLocal)
            {
                CoordType pivot;
                if (_pivotProvider && _pivotProvider(pivot))
                {
                    _activePivot = pivot;
                    _hasActivePivot = true;
                }
                else if (_pivotRequester)
                {
                    // No feature is picking right now: request a pick at the cursor and wait
                    // for its result before rotating around the picked mesh center.
                    _pendingLocalPivot = true;
                    _pivotRequester(xpos, ypos);
                }
            }
            return true;
        }

        if (button == _activeButton)
        {
            _activeAction = CameraAction::None;
            _activeButton = static_cast<int>(CameraMouseButton::None);
            _hasActivePivot = false;
            _pendingLocalPivot = false;
        }
        return false;
    }

    bool OrbitCameraController::processMouseMove(float xpos, float ypos)
    {
        if (_camera == nullptr) return false;

        // A local orbit cannot rotate before its pivot pick resolved; keep the pending drag
        // so the first accepted move applies the accumulated offset.
        if (_activeAction == CameraAction::RotateLocal && !_hasActivePivot && _pendingLocalPivot)
        {
            return false;
        }

        if (_lastXPos < 0.0f || _lastYPos < 0.0f)
        {
            _lastXPos = xpos;
            _lastYPos = ypos;
            return false;
        }

        const float xoffset = xpos - _lastXPos;
        const float yoffset = ypos - _lastYPos;

        bool isChanged = false;
        switch (_activeAction)
        {
        case CameraAction::Pan:
            isChanged = pan(xoffset, yoffset);
            break;
        case CameraAction::Rotate:
            isChanged = rotate(xoffset, yoffset, _camera->getTarget(), false);
            break;
        case CameraAction::RotateLocal:
        {
            const CoordType pivot = _hasActivePivot ? _activePivot : _camera->getTarget();
            isChanged = rotate(xoffset, yoffset, pivot, _hasActivePivot);
            break;
        }
        case CameraAction::Zoom:
            isChanged = applyZoom((_lastYPos - ypos) * kZoomDragStepsPerPixel, xpos, ypos);
            break;
        default:
            break;
        }

        if (isChanged)
        {
            recalcuteCameraPostition();
        }

        _lastXPos = xpos;
        _lastYPos = ypos;
        return isChanged;
    }

    bool OrbitCameraController::processMouseScroll(float yoffset, float xpos, float ypos)
    {
        if (_camera == nullptr) return false;
        return applyZoom(yoffset, xpos, ypos);
    }

    bool OrbitCameraController::pan(float xoffset, float yoffset)
    {
        const float panSensitivity = _panSensitivity * _camera->getMoveSensitivity();
        const float zoom = _camera->getZoom();

        const CoordType delta = (_camera->getRight() * (-xoffset * panSensitivity * zoom)) +
                                (_camera->getUp() * (yoffset * panSensitivity * zoom));
        _camera->setPosition(_camera->getPosition() + delta);
        _camera->setTarget(_camera->getTarget() + delta);
        return true;
    }

    bool OrbitCameraController::rotate(float xoffset, float yoffset, const CoordType &pivot,
                                       bool localAxis)
    {
        const float orbitSensitivity = _orbitSensitivity * _camera->getRotateSensitivity();

        CoordType yawAxis = localAxis ? _camera->getUp() : _camera->getWorldUp();
        CoordType pitchAxis = _camera->getRight();
        yawAxis.normalize();
        pitchAxis.normalize();

        const Quatf yawQuat = Quatf::fromAxisAndAngle(yawAxis, -xoffset * orbitSensitivity);
        const Quatf pitchQuat = Quatf::fromAxisAndAngle(pitchAxis, -yoffset * orbitSensitivity);
        const Quatf rotation = yawQuat * pitchQuat;

        Quatf orientation = rotation * _camera->getOrientation();
        orientation.normalize();
        _camera->setOrientation(orientation);

        const CoordType newFront = orientation.rotateVector(CoordType(0.0f, 0.0f, -1.0f));
        _camera->setFront(newFront);
        _camera->setRigth(orientation.rotateVector(CoordType(1.0f, 0.0f, 0.0f)));
        _camera->setUp(orientation.rotateVector(CoordType(0.0f, 1.0f, 0.0f)));

        // Orbit the eye around the pivot. The global gesture pivots around the target, while
        // the local gesture pivots around the picked mesh center; in both cases the target is
        // rotated rigidly with the eye so the global rotation (and pan/zoom) data that is
        // derived from the target stays consistent afterwards.
        const CoordType position = _camera->getPosition();
        const CoordType target = _camera->getTarget();
        _camera->setPosition(pivot + rotation.rotateVector(position - pivot));
        _camera->setTarget(pivot + rotation.rotateVector(target - pivot));
        return true;
    }

    bool OrbitCameraController::applyZoom(float steps, float cursorX, float cursorY)
    {
        if (steps == 0.0f) return false;

        const float oldZoom = _camera->getZoom();
        const float zoomSpeed = 1.0f + (_zoomSpeed - 1.0f) * _camera->getZoomSensitivity();
        if (zoomSpeed <= 0.0f) return false;

        float newZoom = oldZoom * std::pow(zoomSpeed, -steps);
        if (newZoom < kMinZoom)
        {
            newZoom = kMinZoom;
        }
        if (newZoom == oldZoom) return false;

        if (_zoomAnchor.load() == CameraZoomAnchor::MouseCursor && _viewportWidth > 0.0f &&
            _viewportHeight > 0.0f)
        {
            const float ndcX = (cursorX / _viewportWidth) * 2.0f - 1.0f;
            const float ndcY = 1.0f - (cursorY / _viewportHeight) * 2.0f;
            const float aspect = _viewportWidth / _viewportHeight;

            CoordType right = _camera->getRight();
            CoordType up = _camera->getUp();
            right.normalize();
            up.normalize();

            // Move the view center so the world point under the cursor keeps its screen position.
            const CoordType shift =
                (right * (ndcX * aspect) + up * ndcY) * (oldZoom - newZoom);
            _camera->setPosition(_camera->getPosition() + shift);
            _camera->setTarget(_camera->getTarget() + shift);
        }

        _camera->setZoom(newZoom);
        return true;
    }

    void OrbitCameraController::setInputBindings(const CameraInputBindings &bindings)
    {
        _bindings = bindings;
    }

    void OrbitCameraController::setViewportSize(float width, float height)
    {
        _viewportWidth = width > 0.0f ? width : 1.0f;
        _viewportHeight = height > 0.0f ? height : 1.0f;
    }

    void OrbitCameraController::setLocalPivotProvider(CameraPivotProvider provider)
    {
        _pivotProvider = Common::move(provider);
    }

    void OrbitCameraController::setLocalPivotRequester(CameraPivotRequester requester)
    {
        _pivotRequester = Common::move(requester);
    }

    void OrbitCameraController::setZoomAnchor(CameraZoomAnchor anchor)
    {
        _zoomAnchor.store(anchor);
    }

    CameraZoomAnchor OrbitCameraController::getZoomAnchor() const
    {
        return _zoomAnchor.load();
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
