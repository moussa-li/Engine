#pragma once
#include "Common/Vector.hpp"

namespace EgLab::RE
{
    class Quatf : public Common::Vector4f
    {
    public:
        Quatf(float w, float x, float y, float z) : Common::Vector4f(w, x, y, z)
        {
        }
        static Quatf fromAxisAndAngle(const Common::Vector3f& axis, float angleRad)
        {
            float halfAngle = angleRad * float(0.5);
            float s = std::sin(halfAngle);
            float c = std::cos(halfAngle);

            Common::Vector3f normalizedAxis = axis;
            normalizedAxis.normalize();

            return Quatf(c, normalizedAxis[0] * s, normalizedAxis[1] * s, normalizedAxis[2] * s);
        }

        // const float& operator[](size_t dim) const
        // {
        //     return this->_data[0][dim];
        // }

        // 4. 四元数乘法 (Hamilton Product)
        // 用于叠加旋转，注意四元数乘法不满足交换律！
        Quatf operator*(const Quatf& q) const
        {
            const float &w1 = (*this)[0], x1 = (*this)[1], y1 = (*this)[2], z1 = (*this)[3];
            const float &w2 = q[0], x2 = q[1], y2 = q[2], z2 = q[3];

            return Quatf(
                w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2, w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2,
                w1 * y2 - x1 * z2 + y1 * w2 + z1 * x2, w1 * z2 + x1 * y2 - y1 * x2 + z1 * w2);
        }

        // 5. 用四元数旋转一个 3D 向量 (v' = q * v * q^-1)
        // 这是将相机的局部坐标轴转换到世界坐标系的唯一正确方法
        Common::Vector3f rotateVector(const Common::Vector3f& v) const
        {
            // 提取四元数的向量部分 (x, y, z)
            Common::Vector3f u((*this)[1], (*this)[2], (*this)[3]);

            // 公式展开：v + 2.0 * (u × (u × v + w * v))
            // 这种写法比转换成旋转矩阵再相乘要快得多，且无万向锁
            Common::Vector3f uv = u.cross(v);
            Common::Vector3f uuv = u.cross(uv);

            return v + ((uv * (*this)[0]) + uuv) * float(2.0);
        }
    };
} // namespace EgLab::RE