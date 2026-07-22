#include "Camera.hpp"

#include <math.h>

namespace EgLab
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

    Camera::Camera(const unsigned int &width, const unsigned int &height, CoordType position,
                   CoordType up, float yaw, float pitch)
        : _width(width),
          _height(height),
          _lastX(width * 0.5),
          _lastY(height * 0.5),
          _front(CoordType(0.0f, 0.0f, 1.0f)),
          _position(position),
          _worldUp(up),
          _yaw(yaw),
          _pitch(pitch),
          _movementSpeed(SPEED),
          _mouseSensitivity(SENSITIVITY),
          _zoom(ZOOM)
    {
        updateCameraVectors();
    }

    void Camera::updateCameraVectors()
    {
        CoordType front;
        front.x() = std::cos(radians(_yaw)) * std::cos(radians(_pitch));
        front.y() = std::sin(radians(_pitch));
        front.z() = std::sin(radians(_yaw)) * std::cos(radians(_pitch));
        front.normalize();

        _front = front;
        _right = front.cross(_worldUp);
        _right.normalize();

        _up = _right.cross(_front);
        _up.normalize();
    }

    Matrix4f Camera::perspective() const
    {
        const float fovy = radians(_zoom);
        const float tanHalfFovy = std::tan(fovy * 0.5); // fovy : Field of View Y
        const float aspect = _width / _height;          // maybe could remain. and update by setHW
        static const float zNear = 0.1f;
        static const float zFar = 100.0f;

        Matrix4f result;
        result[0][0] = 1 / (aspect * tanHalfFovy);
        result[1][1] = 1 / (tanHalfFovy);
        result[2][2] = -(zFar + zNear) / (zFar - zNear);
        result[3][2] = -1.0f;
        result[2][3] = -(2 * zFar * zNear) / (zFar - zNear);

        return result;
    }

    Matrix4f Camera::view() const
    {
        CoordType f(_front);
        CoordType s(_front.cross(_up));
        CoordType u(s.cross(f));
        f.normalize();
        s.normalize();
        u.normalize();

        Matrix4f result = Matrix4f::Identity();
        result[0][0] = s.x();
        result[0][1] = s.y();
        result[0][2] = s.z();
        result[1][0] = u.x();
        result[1][1] = u.y();
        result[1][2] = u.z();
        result[2][0] = -f.x();
        result[2][1] = -f.y();
        result[2][2] = -f.z();
        result[0][3] = -s.dot(_position);
        result[1][3] = -u.dot(_position);
        result[2][3] = f.dot(_position);
        result[3][3] = 1.0f;

        return result;
    }

    void Camera::lookAt(const CoordType &position, const CoordType &front, const CoordType &up)
    {
        _position = position;
        _front = front;
        _up = up;
        // updateCameraVectors();
    }

} // namespace EgLab