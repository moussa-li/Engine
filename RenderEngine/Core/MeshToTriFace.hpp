#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/UniquePtr.hpp"
#include "MeshEngine/MeshData/Element.hpp"
#include "RenderEngine/Core/Definites.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"

namespace EgLab::ME
{
    class Mesh;
}

namespace EgLab::RE
{
    /// Converts mesh boundary topology into renderable face and edge index data.
    class RenderEngineAPI MeshToTriFace
    {
    public:
        /// @brief Creates an empty mesh topology translator.
        MeshToTriFace();
        /// @brief Releases the translator's stored topology.
        ~MeshToTriFace();

        /// @brief Sets the mesh whose boundary topology will be translated.
        /// @details The supplied mesh is used to build boundary topology data.
        void setMesh(Common::SharedPtr<ME::Mesh>&);

        /// @brief Returns indices for triangulated boundary faces.
        /// @return Triangle vertex indices.
        Common::DynamicArray<IdxType> getBoundaryFaceIdx() const;

        /// @brief Returns node indices referenced by boundary faces.
        /// @return Boundary face node indices.
        const Common::DynamicArray<IdxType>& getBoundaryFaceNodeIdx() const;

        /// @brief Returns indices for boundary lines.
        /// @return Line endpoint indices.
        Common::DynamicArray<IdxType> getBoundaryLineIdx() const;

        /// @brief Returns element-edge indices for the mesh boundary.
        /// @return Boundary element-edge indices.
        Common::DynamicArray<IdxType> getBoundaryElementEdgeIdx() const;

        /// @brief Returns source element locations for boundary faces.
        /// @return Element and local-face locations corresponding to boundary faces.
        const Common::DynamicArray<ME::ElemFaceLocation>& getBoundaryFaceLocations() const;

        /// @brief Returns source element locations for boundary lines.
        /// @return Element and local-edge locations corresponding to boundary lines.
        const Common::DynamicArray<ME::ElemFaceEdgeLocation>& getBoundaryLineLocations() const;

    private:
        class Impl;
        Common::UniquePtr<Impl> _impl;
    };
} // namespace EgLab::RE