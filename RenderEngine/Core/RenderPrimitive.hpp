#pragma once
#include <cstddef>
#include <limits>

#include "Common/BBox.hpp"
#include "Common/Matrix.hpp"
#include "Common/SharedPtr.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"

namespace EgLab::RE
{
    class Shader;
    class Transform;

    struct PickContext
    {
        Scalar x{0.0f};
        Scalar y{0.0f};
        unsigned int width{0};
        unsigned int height{0};
        Scalar radius{6.0f};
        Common::Matrix4f view;
        Common::Matrix4f projection;
        mutable Scalar bestDepth{std::numeric_limits<Scalar>::max()};
        mutable Scalar bestDistanceSquared{std::numeric_limits<Scalar>::max()};
    };

    namespace PickUtils
    {
        struct ScreenPoint
        {
            Scalar x;
            Scalar y;
            Scalar depth;
        };

        inline bool projectPoint(const CoordType &point, const PickContext &context,
                                 ScreenPoint &screenPoint)
        {
            const Scalar eyeX = context.view[0][0] * point.x() + context.view[0][1] * point.y() +
                                context.view[0][2] * point.z() + context.view[0][3];
            const Scalar eyeY = context.view[1][0] * point.x() + context.view[1][1] * point.y() +
                                context.view[1][2] * point.z() + context.view[1][3];
            const Scalar eyeZ = context.view[2][0] * point.x() + context.view[2][1] * point.y() +
                                context.view[2][2] * point.z() + context.view[2][3];
            const Scalar eyeW = context.view[3][0] * point.x() + context.view[3][1] * point.y() +
                                context.view[3][2] * point.z() + context.view[3][3];
            const Scalar clipX = context.projection[0][0] * eyeX + context.projection[0][1] * eyeY +
                                 context.projection[0][2] * eyeZ + context.projection[0][3] * eyeW;
            const Scalar clipY = context.projection[1][0] * eyeX + context.projection[1][1] * eyeY +
                                 context.projection[1][2] * eyeZ + context.projection[1][3] * eyeW;
            const Scalar clipZ = context.projection[2][0] * eyeX + context.projection[2][1] * eyeY +
                                 context.projection[2][2] * eyeZ + context.projection[2][3] * eyeW;
            const Scalar clipW = context.projection[3][0] * eyeX + context.projection[3][1] * eyeY +
                                 context.projection[3][2] * eyeZ + context.projection[3][3] * eyeW;
            if (clipW <= 1e-6f)
            {
                return false;
            }

            const Scalar ndcX = clipX / clipW;
            const Scalar ndcY = clipY / clipW;
            const Scalar ndcZ = clipZ / clipW;
            if (ndcZ < -1.0f || ndcZ > 1.0f)
            {
                return false;
            }

            screenPoint.x = (ndcX + 1.0f) * 0.5f * context.width;
            screenPoint.y = (1.0f - ndcY) * 0.5f * context.height;
            screenPoint.depth = ndcZ;
            return true;
        }
    } // namespace PickUtils

    class RenderEngineAPI RenderPrimitive
    {
    public:
        virtual ~RenderPrimitive();
        virtual void setup() = 0;
        virtual void draw(Common::SharedPtr<Shader> shader) = 0;
        virtual Common::BBox<Scalar, 3> getBounds() const = 0;
        virtual void drawHighlight(Common::SharedPtr<Shader>, IdxType)
        {
        }

        virtual IdxType touch(const PickContext &)
        {
            return static_cast<IdxType>(-1);
        };
    };
} // namespace EgLab::RE