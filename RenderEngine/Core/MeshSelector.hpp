#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "RenderEngine/Core/MeshToTriFace.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"
#include "RenderEngine/Core/Selector.hpp"

namespace EgLab::RE
{
    /// Maps primitive picking results back to entities in a MeshEngine mesh.
    class RenderEngineAPI MeshSelector : public Selector
    {
    public:
        /**
         * @brief Gets the indices to highlight for a picked primitive element.
         * @param primitive Primitive that produced the pick.
         * @param pickedIndex Index returned by the primitive pick operation.
         * @return Indices used to highlight the selected mesh element.
         */
        Common::DynamicArray<IdxType> getHighlightIndices(
            const Common::SharedPtr<RenderPrimitive>& primitive,
            IdxType pickedIndex) const override;
        /// @brief Sets the callback invoked with resolved mesh pick information.
        /// @param callback Callback to register.
        void registerPickInfoCallback(PickInfoCallback callback) override;
        /// @brief Resolves and reports a pick from a registered primitive.
        /// @param primitive Primitive that produced the pick.
        /// @param pickedIndex Index returned by the primitive pick operation.
        void processPick(const Common::SharedPtr<RenderPrimitive>& primitive,
                         IdxType pickedIndex) override;
        /**
         * @brief Associates mesh primitives with the mesh topology they represent.
         * @param mesh Source mesh.
         * @param translator Translator containing the mesh boundary topology.
         * @param nodePrimitive Primitive used to render mesh nodes.
         * @param linePrimitive Primitive used to render mesh edges.
         * @param facePrimitive Primitive used to render mesh faces.
         */
        void registerMesh(const Common::SharedPtr<ME::Mesh>& mesh,
                          const Common::SharedPtr<MeshToTriFace>& translator,
                          const Common::SharedPtr<RenderPrimitive>& nodePrimitive,
                          const Common::SharedPtr<RenderPrimitive>& linePrimitive,
                          const Common::SharedPtr<RenderPrimitive>& facePrimitive);
        /// @brief Removes the registration for a mesh.
        /// @param mesh Address of the mesh to unregister.
        void unregisterMesh(const ME::Mesh* mesh);

        /// @brief Releases this selector and its registrations.
        ~MeshSelector();

    private:
        struct MeshRegistration
        {
            Common::SharedPtr<ME::Mesh> mesh;
            Common::SharedPtr<MeshToTriFace> translator;
            Common::SharedPtr<RenderPrimitive> nodePrimitive;
            Common::SharedPtr<RenderPrimitive> linePrimitive;
            Common::SharedPtr<RenderPrimitive> facePrimitive;
        };

        const MeshRegistration* findRegistration(
            const Common::SharedPtr<RenderPrimitive>& primitive) const;

        Common::DynamicArray<MeshRegistration> _registrations;
        PickInfoCallback _pickInfoCallback;
        PrimitivePickInfo _lastPickInfo;
        const ME::Mesh* _lastPickedMesh{nullptr};
        bool _hasLastPickInfo{false};
    };
} // namespace EgLab::RE