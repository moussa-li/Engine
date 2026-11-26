#pragma once
#include "Common/Vector.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "RenderEngine/Core/MeshToTriFace.hpp"
#include "RenderEngine/Core/PrimitiveCreator.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"

namespace EgLab::RE
{
    /// Creates render primitives from a MeshEngine mesh and its translated topology.
    class RenderEngineAPI MeshPrimitiveCreator : public PrimitiveCreator
    {
    public:
        /// @brief Creates a primitive creator for the specified mesh.
        /// @details The source mesh is used to populate render primitives.
        MeshPrimitiveCreator(Common::SharedPtr<ME::Mesh>);

        /// @brief Creates a primitive creator that reuses an existing topology translator.
        /// @details Used to rebuild face and line primitives after the translator's hidden
        /// element set changes, without recomputing a new translator.
        MeshPrimitiveCreator(Common::SharedPtr<ME::Mesh>, Common::SharedPtr<MeshToTriFace>);

        /// @brief Releases the mesh translator and associated state.
        ~MeshPrimitiveCreator();

        /// @brief Returns the topology translator used for the source mesh.
        /// @return The mesh-to-triangle-face translator.
        Common::SharedPtr<MeshToTriFace> getTranslator() const;

        /// @brief Source of the index data drawn by the line primitive.
        enum class LineIndexSource
        {
            /// Every triangle edge of the tessellation (default, dense mesh wireframe).
            AllTriangleEdges,
            /// Only the outer boundary of the tessellated patches (face outline).
            BoundaryOnly,
        };

        /// @brief Selects which line indices the line primitive receives.
        /// @details Must be called before getPrimitive<RenderLine>().
        void setLineIndexSource(LineIndexSource source);

        /// @brief Enables smooth (node-averaged) normals for the face primitive.
        /// @details Must be called before getPrimitive<RenderFace>(). Normals are averaged per
        ///          mesh node, so triangles of one B-Rep face shade smoothly while different
        ///          faces keep their sharp boundary.
        void setSmoothNormals(bool smooth);

        /// @brief Supplies one surface UV per source mesh node for the face primitive.
        /// @details The buffer must outlive getPrimitive<RenderFace>(). UVs are expanded per
        ///          face corner in the same order as the face vertices, so the UV shader can
        ///          interpolate them across each triangle.
        /// @param uvs Per-mesh-node UV table, or null when the mesh has no surface UVs.
        void setNodeUvs(const Common::DynamicArray<Common::Vector2f>* uvs);

    protected:
        /// @brief Populates a node primitive from the source mesh.
        virtual void updateData(Common::SharedPtr<RenderNode>);
        /// @brief Populates a line primitive from the source mesh.
        virtual void updateData(Common::SharedPtr<RenderLine>);
        /// @brief Populates a face primitive from the source mesh.
        virtual void updateData(Common::SharedPtr<RenderFace>);

    private:
        Common::SharedPtr<ME::Mesh> _mesh;
        Common::SharedPtr<MeshToTriFace> _translator;
        LineIndexSource _lineIndexSource{LineIndexSource::AllTriangleEdges};
        bool _smoothNormals{false};
        const Common::DynamicArray<Common::Vector2f>* _nodeUvs{nullptr};
    };
} // namespace EgLab::RE