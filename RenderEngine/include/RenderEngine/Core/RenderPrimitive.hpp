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
        /// Cursor x-coordinate in viewport pixels.
        Scalar x{0.0f};
        /// Cursor y-coordinate in viewport pixels.
        Scalar y{0.0f};
        /// Viewport width in pixels.
        unsigned int width{0};
        /// Viewport height in pixels.
        unsigned int height{0};
        /// Maximum screen-space picking distance in pixels.
        Scalar radius{6.0f};
        /// View matrix used to project world-space points.
        Common::Matrix4f view;
        /// Projection matrix used to project world-space points.
        Common::Matrix4f projection;
        /// Best depth found so far during a pick query.
        mutable Scalar bestDepth{std::numeric_limits<Scalar>::max()};
        /// Best squared screen-space distance found so far.
        mutable Scalar bestDistanceSquared{std::numeric_limits<Scalar>::max()};
    };

    namespace PickUtils
    {
        struct ScreenPoint
        {
            /// Horizontal screen coordinate in pixels.
            Scalar x;
            /// Vertical screen coordinate in pixels.
            Scalar y;
            /// Normalized device depth.
            Scalar depth;
        };

        /**
         * @brief Projects a world-space point into viewport coordinates.
         * @param point World-space point to project.
         * @param context Viewport dimensions and camera matrices.
         * @param screenPoint Receives projected coordinates on success.
         * @return true if the point is in front of the camera and within the depth range.
         */
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

    /// Abstract base class for renderable and pickable geometry primitives.
    class RenderEngineAPI RenderPrimitive
    {
    public:
        /// @brief Releases resources owned by the primitive.
        virtual ~RenderPrimitive();
        /// @brief Initializes the primitive's rendering resources.
        virtual void setup() = 0;
        /// @brief Draws the primitive using the specified shader.
        /// @param shader Shader used for rendering.
        virtual void draw(Common::SharedPtr<Shader> shader) = 0;
        /// @brief Returns the world-space bounds of the primitive.
        virtual Common::BBox<Scalar, 3> getBounds() const = 0;
        /// @brief Draws a highlighted element; the default implementation does nothing.
        virtual void drawHighlight(Common::SharedPtr<Shader>, IdxType)
        {
        }

        /// @brief Tests the primitive for a pick; the default implementation reports no hit.
        /// @return The invalid index when picking is not implemented.
        virtual IdxType touch(const PickContext &)
        {
            return static_cast<IdxType>(-1);
        };
    };
} // namespace EgLab::RE