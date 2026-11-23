#pragma once

#include "Common/BBox.hpp"
#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Vector.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"

namespace EgLab::RE
{
    class Shader;
    class VertexArray;
    class VertexBuffer;

    /// Placement of the three orthogonal reference planes inside the scene bounds.
    enum class ReferencePlanePlacement
    {
        /// Planes pass through the bounding-box center along each axis.
        BoundingBoxCenter,
        /// Planes lie on the three back faces (minimum extent) of the bounding box.
        BoundingBoxBackFaces,
    };

    /// Appearance of the three orthogonal reference planes.
    struct ReferencePlaneSettings
    {
        /// Whether the reference planes are rendered.
        bool enabled{true};
        /// Whether grid lines are drawn on top of the plane fill.
        bool showGrid{true};
        /// Where the planes are placed inside the scene bounds.
        ReferencePlanePlacement placement{ReferencePlanePlacement::BoundingBoxBackFaces};
        /// Scale applied to the in-plane extents about the scene bounds center. Placement
        /// shifts each plane along its normal so back-face planes meet at one bounds vertex.
        float scale{1.2f};
        /// Per-plane RGBA color. Index 0 = YZ plane (X normal), 1 = XZ plane (Y normal),
        /// 2 = XY plane (Z normal).
        Common::Vector4f colors[3]{{1.0f, 0.3f, 0.3f, 0.1f}, {0.3f, 1.0f, 0.3f, 0.1f},
                                   {0.3f, 0.5f, 1.0f, 0.1f}};
    };

    /// Draws the three Cartesian reference planes (YZ, XZ and XY) sized to the scene bounds.
    /// Each plane is rendered as a translucent quad plus optional grid lines.
    class RenderEngineAPI ReferencePlane
    {
    public:
        /// @brief Creates an empty reference plane renderer.
        ReferencePlane();
        /// @brief Releases the reference plane GPU resources.
        ~ReferencePlane();

        /// @brief Returns the current appearance settings.
        const ReferencePlaneSettings &getSettings() const;
        /// @brief Replaces the appearance settings.
        /// @param settings New appearance settings.
        void setSettings(const ReferencePlaneSettings &settings);

        /// @brief Draws the three planes for the supplied scene bounds.
        /// @details The caller must have uploaded the projection and view matrices to the
        ///          shader before calling this function.
        /// @param bounds World-space bounds the planes span.
        /// @param shader Shader used to draw the planes.
        void draw(const Common::BBox<Scalar, 3> &bounds,
                  const Common::SharedPtr<Shader> &shader);

    private:
        /// Vertex range of one plane inside the shared quad/line buffers.
        struct PlaneRange
        {
            size_t offset{0};
            size_t count{0};
        };

        void rebuild(const Common::BBox<Scalar, 3> &bounds);
        void uploadQuadGeometry();
        void uploadLineGeometry();
        bool boundsChanged(const Common::BBox<Scalar, 3> &bounds) const;
        static Scalar computeGridStep(Scalar maxExtent);

    private:
        ReferencePlaneSettings _settings;

        Common::DynamicArray<CoordType> _quadVertices;
        Common::DynamicArray<CoordType> _lineVertices;
        PlaneRange _quadRanges[3];
        PlaneRange _lineRanges[3];

        Common::SharedPtr<VertexArray> _quadVertexArray;
        Common::SharedPtr<VertexBuffer> _quadVertexBuffer;
        Common::SharedPtr<VertexArray> _lineVertexArray;
        Common::SharedPtr<VertexBuffer> _lineVertexBuffer;

        CoordType _cachedMin;
        CoordType _cachedMax;
        bool _hasCachedBounds{false};
        bool _cachedShowGrid{true};
        ReferencePlanePlacement _cachedPlacement{ReferencePlanePlacement::BoundingBoxBackFaces};
        float _cachedScale{1.2f};
    };
} // namespace EgLab::RE
