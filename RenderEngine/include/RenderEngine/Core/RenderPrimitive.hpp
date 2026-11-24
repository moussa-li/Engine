#pragma once
#include <cmath>
#include <cstddef>
#include <limits>

#include "Common/BBox.hpp"
#include "Common/DynamicArray.hpp"
#include "Common/Matrix.hpp"
#include "Common/SharedPtr.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/PickKind.hpp"
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
        /// Priority of the best hit; used to prefer nodes over edges over faces.
        mutable int bestPriority{0};

        /// Compares a candidate hit with the best hit recorded so far. A per-kind depth
        /// offset expands the near-tie window in favour of higher priorities.
        bool acceptHit(PickKind kind, Scalar depth, Scalar distanceSquared) const
        {
            const Scalar biasedDepth = depth + pickDepthBias(kind);
            const Scalar delta = biasedDepth - bestDepth;
            if (delta > kPickDepthEpsilon)
            {
                return false;
            }

            if (delta >= -kPickDepthEpsilon)
            {
                const int priority = pickPriority(kind);
                if (priority < bestPriority)
                {
                    return false;
                }
                if (priority == bestPriority && distanceSquared >= bestDistanceSquared)
                {
                    return false;
                }
            }

            bestDepth = biasedDepth;
            bestDistanceSquared = distanceSquared;
            bestPriority = pickPriority(kind);
            return true;
        }
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
         * @brief Projects a world-space point with a concatenated view-projection matrix.
         * @param point World-space point to project.
         * @param viewProjection Concatenated projection * view matrix.
         * @param width Viewport width in pixels.
         * @param height Viewport height in pixels.
         * @param screenPoint Receives projected coordinates on success.
         * @return true if the point is in front of the camera and within the depth range.
         */
        inline bool projectPoint(const CoordType &point, const Common::Matrix4f &viewProjection,
                                 unsigned int width, unsigned int height,
                                 ScreenPoint &screenPoint)
        {
            const Scalar clipX = viewProjection[0][0] * point.x() +
                                 viewProjection[0][1] * point.y() +
                                 viewProjection[0][2] * point.z() + viewProjection[0][3];
            const Scalar clipY = viewProjection[1][0] * point.x() +
                                 viewProjection[1][1] * point.y() +
                                 viewProjection[1][2] * point.z() + viewProjection[1][3];
            const Scalar clipZ = viewProjection[2][0] * point.x() +
                                 viewProjection[2][1] * point.y() +
                                 viewProjection[2][2] * point.z() + viewProjection[2][3];
            const Scalar clipW = viewProjection[3][0] * point.x() +
                                 viewProjection[3][1] * point.y() +
                                 viewProjection[3][2] * point.z() + viewProjection[3][3];
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

            screenPoint.x = (ndcX + 1.0f) * 0.5f * width;
            screenPoint.y = (1.0f - ndcY) * 0.5f * height;
            screenPoint.depth = ndcZ;
            return true;
        }

        /// @brief Projects a world-space point using separate view and projection matrices.
        inline bool projectPoint(const CoordType &point, const Common::Matrix4f &view,
                                 const Common::Matrix4f &projection, unsigned int width,
                                 unsigned int height, ScreenPoint &screenPoint)
        {
            return projectPoint(point, projection * view, width, height, screenPoint);
        }

        /// @brief Projects a world-space point using a single-point pick context.
        inline bool projectPoint(const CoordType &point, const PickContext &context,
                                 ScreenPoint &screenPoint)
        {
            return projectPoint(point, context.view, context.projection, context.width,
                                context.height, screenPoint);
        }
    } // namespace PickUtils

    /// Screen-space rectangle used by rubber-band selection. Coordinates are expressed in
    /// framebuffer pixels with the origin at the top-left corner, matching PickContext.
    struct RectPickContext
    {
        /// Left edge of the rectangle in viewport pixels.
        Scalar minX{0.0f};
        /// Top edge of the rectangle in viewport pixels.
        Scalar minY{0.0f};
        /// Right edge of the rectangle in viewport pixels.
        Scalar maxX{0.0f};
        /// Bottom edge of the rectangle in viewport pixels.
        Scalar maxY{0.0f};
        /// Viewport width in pixels.
        unsigned int width{0};
        /// Viewport height in pixels.
        unsigned int height{0};
        /// View matrix used to project world-space points.
        Common::Matrix4f view;
        /// Projection matrix used to project world-space points.
        Common::Matrix4f projection;
        /// True selects entities completely inside the rectangle; false selects any entity
        /// intersecting the rectangle.
        bool containsOnly{true};

        /// @brief Reports whether a screen-space point lies inside the rectangle.
        bool contains(Scalar x, Scalar y) const
        {
            return x >= minX && x <= maxX && y >= minY && y <= maxY;
        }

        /// @brief Reports whether a screen-space segment intersects the rectangle.
        /// @details Uses the Liang-Barsky clipping test so segments crossing the rectangle
        ///          without an endpoint inside are detected as well.
        bool segmentIntersects(Scalar x0, Scalar y0, Scalar x1, Scalar y1) const

        {
            if (contains(x0, y0) || contains(x1, y1))
            {
                return true;
            }

            const Scalar deltaX = x1 - x0;
            const Scalar deltaY = y1 - y0;
            Scalar tMin = 0.0f;
            Scalar tMax = 1.0f;

            const auto clip = [&tMin, &tMax](Scalar p, Scalar q) -> bool {
                if (std::abs(p) <= 1e-8f)
                {
                    return q >= 0.0f;
                }
                const Scalar ratio = q / p;
                if (p < 0.0f)
                {
                    if (ratio > tMax)
                    {
                        return false;
                    }
                    if (ratio > tMin)
                    {
                        tMin = ratio;
                    }
                }
                else
                {
                    if (ratio < tMin)
                    {
                        return false;
                    }
                    if (ratio < tMax)
                    {
                        tMax = ratio;
                    }
                }
                return true;
            };

            if (!clip(-deltaX, x0 - minX) || !clip(deltaX, maxX - x0) ||
                !clip(-deltaY, y0 - minY) || !clip(deltaY, maxY - y0))
            {
                return false;
            }
            return tMin <= tMax;
        }

        /// @brief Reports whether a screen-space triangle intersects the rectangle.
        /// @details A triangle intersects the rectangle when a vertex is inside, an edge
        ///          crosses the rectangle, or the rectangle lies completely inside the
        ///          triangle.
        bool triangleIntersects(Scalar ax, Scalar ay, Scalar bx, Scalar by, Scalar cx,
                                Scalar cy) const
        {
            if (contains(ax, ay) || contains(bx, by) || contains(cx, cy))
            {
                return true;
            }
            if (segmentIntersects(ax, ay, bx, by) || segmentIntersects(bx, by, cx, cy) ||
                segmentIntersects(cx, cy, ax, ay))
            {
                return true;
            }
            return pointInTriangle(minX, minY, ax, ay, bx, by, cx, cy) ||
                   pointInTriangle(maxX, minY, ax, ay, bx, by, cx, cy) ||
                   pointInTriangle(maxX, maxY, ax, ay, bx, by, cx, cy) ||
                   pointInTriangle(minX, maxY, ax, ay, bx, by, cx, cy);
        }

        /// @brief Reports whether a screen-space point lies inside a triangle.
        static bool pointInTriangle(Scalar px, Scalar py, Scalar ax, Scalar ay, Scalar bx,
                                    Scalar by, Scalar cx, Scalar cy)
        {
            const Scalar area = (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
            if (std::abs(area) <= 1e-8f)
            {
                return false;
            }

            const Scalar d0 = (px - ax) * (by - ay) - (py - ay) * (bx - ax);
            const Scalar d1 = (px - bx) * (cy - by) - (py - by) * (cx - bx);
            const Scalar d2 = (px - cx) * (ay - cy) - (py - cy) * (ax - cx);
            const bool hasNegative = d0 < -1e-6f || d1 < -1e-6f || d2 < -1e-6f;
            const bool hasPositive = d0 > 1e-6f || d1 > 1e-6f || d2 > 1e-6f;
            return !(hasNegative && hasPositive);
        }

        /// @brief Concatenates view and projection once so repeated projections stay cheap.
        void prepare() const
        {
            if (_prepared)
            {
                return;
            }
            _viewProjection = projection * view;
            _prepared = true;
        }

        /// @brief Projects a world-space point using the cached view-projection matrix.
        /// @return true when the point is in front of the camera and inside the depth range.
        bool project(const CoordType &point, PickUtils::ScreenPoint &screenPoint) const
        {
            prepare();
            return PickUtils::projectPoint(point, _viewProjection, width, height, screenPoint);
        }

    private:
        mutable Common::Matrix4f _viewProjection;
        mutable bool _prepared{false};
    };

    namespace PickUtils
    {
        /// @brief Projects a world-space point using a rectangle pick context.
        inline bool projectPoint(const CoordType &point, const RectPickContext &context,
                                 ScreenPoint &screenPoint)
        {
            return projectPoint(point, context.view, context.projection, context.width,
                                context.height, screenPoint);
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

        /// @brief Draws many highlighted elements at once.
        /// @param shader Shader used for highlighting.
        /// @param indices Element indices to highlight.
        /// @details The default implementation replays drawHighlight() per index, which costs
        ///          one draw call per element. Primitives that can batch override this to keep
        ///          large selections cheap.
        virtual void drawHighlightBatch(Common::SharedPtr<Shader> shader,
                                        const Common::DynamicArray<IdxType> &indices)
        {
            for (size_t index = 0; index < indices.size(); ++index)
            {
                drawHighlight(shader, indices[index]);
            }
        }

        /// @brief Tests the primitive for a pick; the default implementation reports no hit.
        /// @return The invalid index when picking is not implemented.
        virtual IdxType touch(const PickContext &)
        {
            return static_cast<IdxType>(-1);
        };

        /// @brief Collects the primitive elements covered by a rubber-band rectangle.
        /// @param context Screen-space rectangle and camera matrices.
        /// @param outIndices Receives the indices of every matching element.
        /// @details The default implementation reports no matches.
        virtual void touchRect(const RectPickContext &, Common::DynamicArray<IdxType> &) const
        {
        }

        /// @brief Kind of entity this primitive represents for picking.
        /// @return The primitive's pick kind, or PickKind::None when it cannot be picked.
        virtual PickKind getPickKind() const
        {
            return PickKind::None;
        }

        /// @brief Sets whether the renderer draws this primitive and picking can see it.
        void setVisible(bool visible)
        {
            _visible = visible;
            _pickable = visible;
        }

        /// @brief Reports whether the primitive participates in drawing.
        bool isVisible() const
        {
            return _visible;
        }

        /// @brief Overrides picking independently of drawing.
        /// @details Hidden geometry faces stay pickable so a wireframe view still resolves the
        ///          feature under the cursor.
        void setPickable(bool pickable)
        {
            _pickable = pickable;
        }

        /// @brief Reports whether picking may consider the primitive.
        bool isPickable() const
        {
            return _pickable;
        }

    private:
        bool _visible{true};
        bool _pickable{true};
    };
} // namespace EgLab::RE