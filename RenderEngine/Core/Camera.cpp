#include "Camera.hpp"

#include <math.h>

#include <algorithm>


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

    Camera::Camera(const unsigned int &width, const unsigned int &height, CoordType position,
                   CoordType up, float yaw, float pitch)
        : _width(width),
          _height(height),
          _lastX(width * 0.5),
          _lastY(height * 0.5),
          _front(CoordType(0.0f, 0.0f, -1.0f)),
          _target(CoordType(0.0f, 0.0f, 0.0f)),
          _position(position),
          _orientation(Quatf(1.0f, 0.0f, 0.0f, 0.0f)),
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

    void Camera::setFront(const CoordType &front)
    {
        _front = front;
        //_front.normalize();
    }

    void Camera::setWH(unsigned int width, unsigned int height)
    {
        if (width == 0 || height == 0) return;

        _width = width;
        _height = height;
    }

    void Camera::fitView(const Common::BBox<Scalar, 3> &bounds, float margin)
    {
        if (bounds.isEmpty()) return;

        const CoordType min = bounds.min();
        const CoordType max = bounds.max();
        const CoordType center((min.x() + max.x()) * 0.5f, (min.y() + max.y()) * 0.5f,
                               (min.z() + max.z()) * 0.5f);

        CoordType front(_front);
        CoordType right(_right);
        CoordType up(_up);
        front.normalize();
        right.normalize();
        up.normalize();

        float horizontalExtent = 0.0f;
        float verticalExtent = 0.0f;
        float depthExtent = 0.0f;
        float radius = 0.0f;

        for (int x = 0; x < 2; ++x)
        {
            for (int y = 0; y < 2; ++y)
            {
                for (int z = 0; z < 2; ++z)
                {
                    const CoordType corner(x ? max.x() : min.x(), y ? max.y() : min.y(),
                                           z ? max.z() : min.z());
                    const CoordType offset = corner - center;
                    horizontalExtent = std::max(horizontalExtent, std::abs(offset.dot(right)));
                    verticalExtent = std::max(verticalExtent, std::abs(offset.dot(up)));
                    depthExtent = std::max(depthExtent, std::abs(offset.dot(front)));
                    radius = std::max(radius, offset.length());
                }
            }
        }

        const float safeMargin = std::max(1.0f, margin);
        const float aspect = static_cast<float>(_width) / static_cast<float>(std::max(1u, _height));
        const float halfHeight =
            std::max(verticalExtent, horizontalExtent / std::max(aspect, 0.01f));

        _target = center;
        _zoom = std::max(0.1f, halfHeight * safeMargin);
        _position = center - front * std::max(1.0f, std::max(depthExtent + 1.0f, radius * 2.0f));
        _front = front;
        _right = right;
        _up = up;
    }

    Common::Matrix4f Camera::perspective() const
    {
        return perspective(Common::BBox<Scalar, 3>());
    }

    Common::Matrix4f Camera::perspective(const Common::BBox<Scalar, 3> &bounds) const
    {
#if 0
        const float fovy = radians(_zoom);
        const float tanHalfFovy = std::tan(fovy * 0.5); // fovy : Field of View Y
        const float aspect = _width / _height;          // maybe could remain. and update by setHW
        static const float zNear = 0.1f;
        static const float zFar = 100.0f;

        Common::Matrix4f result;
        result[0][0] = 1 / (aspect * tanHalfFovy);
        result[1][1] = 1 / (tanHalfFovy);
        result[2][2] = -(zFar + zNear) / (zFar - zNear);
        result[3][2] = -1.0f;
        result[2][3] = -(2 * zFar * zNear) / (zFar - zNear);

        return result;
#else

        float halfHeight = _zoom;
        float halfWidth = halfHeight * (static_cast<float>(_width) / _height); // 保持宽高比

        float left = -halfWidth;
        float right = halfWidth;
        float bottom = -halfHeight;
        float top = halfHeight;

        float zNear = 0.01f;
        float zFar = 10000.0f;

        if (!bounds.isEmpty())
        {
            const CoordType min = bounds.min();
            const CoordType max = bounds.max();
            float minDepth = INFINITY;
            float maxDepth = 0.0f;

            for (int x = 0; x < 2; ++x)
            {
                for (int y = 0; y < 2; ++y)
                {
                    for (int z = 0; z < 2; ++z)
                    {
                        const CoordType corner(x ? max.x() : min.x(), y ? max.y() : min.y(),
                                               z ? max.z() : min.z());
                        const float depth = (corner - _position).dot(_front);
                        minDepth = std::min(minDepth, depth);
                        maxDepth = std::max(maxDepth, depth);
                    }
                }
            }

            if (maxDepth > 0.0f)
            {
                zNear = std::max(0.01f, minDepth * 0.9f);
                zFar = std::max(zNear + 0.1f, maxDepth * 1.1f);
            }
        }

        Common::Matrix4f result; // 注意：正交矩阵初始化为 0，而不是单位矩阵

        result[0][0] = 2.0f / (right - left);
        result[1][1] = 2.0f / (top - bottom);
        result[2][2] = -2.0f / (zFar - zNear);
        result[2][3] = -(zFar + zNear) / (zFar - zNear);
        result[3][3] = 1.0f;

        return result;
#endif
    }

    Common::Matrix4f Camera::view() const
    {
        CoordType f(_front);
        CoordType s(_front.cross(_up));
        CoordType u(s.cross(f));
        f.normalize();
        s.normalize();
        u.normalize();

        Common::Matrix4f result = Common::Matrix4f::Identity();
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

} // namespace EgLab::RE