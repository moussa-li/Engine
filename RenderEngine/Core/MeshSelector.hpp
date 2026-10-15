#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "RenderEngine/Core/MeshToTriFace.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"
#include "RenderEngine/Core/Selector.hpp"

namespace EgLab::RE
{
    class RenderEngineAPI MeshSelector : public Selector
    {
    public:
        Common::DynamicArray<IdxType> getHighlightIndices(
            const Common::SharedPtr<RenderPrimitive>& primitive,
            IdxType pickedIndex) const override;
        void registerPickInfoCallback(PickInfoCallback callback) override;
        void processPick(const Common::SharedPtr<RenderPrimitive>& primitive,
                         IdxType pickedIndex) override;
        void registerMesh(const Common::SharedPtr<ME::Mesh>& mesh,
                          const Common::SharedPtr<MeshToTriFace>& translator,
                          const Common::SharedPtr<RenderPrimitive>& nodePrimitive,
                          const Common::SharedPtr<RenderPrimitive>& linePrimitive,
                          const Common::SharedPtr<RenderPrimitive>& facePrimitive);
        void unregisterMesh(const ME::Mesh* mesh);

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