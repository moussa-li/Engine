#pragma once
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

        /// @brief Releases the mesh translator and associated state.
        ~MeshPrimitiveCreator();

        /// @brief Returns the topology translator used for the source mesh.
        /// @return The mesh-to-triangle-face translator.
        Common::SharedPtr<MeshToTriFace> getTranslator() const;

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
    };
} // namespace EgLab::RE