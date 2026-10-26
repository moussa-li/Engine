#include "RenderEngine/Core/MeshSelector.hpp"

namespace EgLab::RE
{
    void MeshSelector::registerMesh(const Common::SharedPtr<ME::Mesh>& mesh,
                                    const Common::SharedPtr<MeshToTriFace>& translator,
                                    const Common::SharedPtr<RenderPrimitive>& nodePrimitive,
                                    const Common::SharedPtr<RenderPrimitive>& linePrimitive,
                                    const Common::SharedPtr<RenderPrimitive>& facePrimitive)
    {
        if (!mesh || !translator)
        {
            return;
        }

        unregisterMesh(mesh.get());
        MeshRegistration registration;
        registration.mesh = mesh;
        registration.translator = translator;
        registration.nodePrimitive = nodePrimitive;
        registration.linePrimitive = linePrimitive;
        registration.facePrimitive = facePrimitive;
        _registrations.pushBack(Common::move(registration));
    }

    void MeshSelector::unregisterMesh(const ME::Mesh* mesh)
    {
        for (size_t index = 0; index < _registrations.size(); ++index)
        {
            auto& registration = _registrations[index];
            if (registration.mesh.get() == mesh)
            {
                if (_lastPickedMesh == mesh)
                {
                    _lastPickInfo = PrimitivePickInfo{};
                    _lastPickedMesh = nullptr;
                    _hasLastPickInfo = false;
                    if (_pickInfoCallback)
                    {
                        _pickInfoCallback(_lastPickInfo);
                    }
                }
                registration.mesh = nullptr;
                registration.translator = nullptr;
                registration.nodePrimitive = nullptr;
                registration.linePrimitive = nullptr;
                registration.facePrimitive = nullptr;
            }
        }
    }

    Common::DynamicArray<IdxType> MeshSelector::getHighlightIndices(
        const Common::SharedPtr<RenderPrimitive>& primitive, IdxType pickedIndex) const
    {
        Common::DynamicArray<IdxType> highlightIndices;
        const MeshRegistration* registration = findRegistration(primitive);
        if (registration != nullptr && primitive == registration->facePrimitive)
        {
            const auto& faceLocations = registration->translator->getBoundaryFaceLocations();
            if (pickedIndex < faceLocations.size())
            {
                const auto& pickedLocation = faceLocations[pickedIndex];
                for (IdxType index = 0; index < faceLocations.size(); ++index)
                {
                    const auto& location = faceLocations[index];
                    if (location.elemId == pickedLocation.elemId &&
                        location.faceIdx == pickedLocation.faceIdx)
                    {
                        highlightIndices.pushBack(index);
                    }
                }
            }
        }

        if (highlightIndices.empty())
        {
            highlightIndices.pushBack(pickedIndex);
        }
        return highlightIndices;
    }

    void MeshSelector::registerPickInfoCallback(PickInfoCallback callback)
    {
        _pickInfoCallback = Common::move(callback);
        if (_pickInfoCallback && _hasLastPickInfo)
        {
            _pickInfoCallback(_lastPickInfo);
        }
    }

    void MeshSelector::processPick(const Common::SharedPtr<RenderPrimitive>& primitive,
                                   IdxType pickedIndex)
    {
        PrimitivePickInfo info;
        const MeshRegistration* registration = findRegistration(primitive);
        if (registration != nullptr && registration->mesh)
        {
            if (primitive == registration->nodePrimitive)
            {
                ME::MeshIterator iterator(*registration->mesh);
                for (IdxType index = 0; index <= pickedIndex; ++index)
                {
                    auto& node = iterator.currentNode();
                    if (index == pickedIndex)
                    {
                        info.kind = PrimitivePickKind::Node;
                        info.entityId = node.getId();
                        info.position = CoordType(node.x(), node.y(), node.z());
                        break;
                    }
                    if (!iterator.hasMoreNode())
                    {
                        break;
                    }
                }
            }
            else if (primitive == registration->linePrimitive)
            {
                const auto& locations = registration->translator->getBoundaryLineLocations();
                if (pickedIndex < locations.size())
                {
                    const auto& location = locations[pickedIndex];
                    const auto& nodeIds =
                        registration->mesh->getElemById(location.elemId).getNodes();
                    info.kind = PrimitivePickKind::Edge;
                    info.entityId = location.elemId;
                    info.faceIdx = location.faceIdx;
                    info.edgeIdx = location.edgeIdx;
                    info.elementNodeCount = nodeIds.size() < info.elementNodeIds.size()
                                                ? nodeIds.size()
                                                : info.elementNodeIds.size();
                    for (size_t index = 0; index < info.elementNodeCount; ++index)
                    {
                        info.elementNodeIds[index] = nodeIds[index];
                    }
                }
            }
            else if (primitive == registration->facePrimitive)
            {
                const auto& locations = registration->translator->getBoundaryFaceLocations();
                if (pickedIndex < locations.size())
                {
                    const auto& location = locations[pickedIndex];
                    const auto& nodeIds =
                        registration->mesh->getElemById(location.elemId).getNodes();
                    info.kind = PrimitivePickKind::Face;
                    info.entityId = location.elemId;
                    info.faceIdx = location.faceIdx;
                    info.elementNodeCount = nodeIds.size() < info.elementNodeIds.size()
                                                ? nodeIds.size()
                                                : info.elementNodeIds.size();
                    for (size_t index = 0; index < info.elementNodeCount; ++index)
                    {
                        info.elementNodeIds[index] = nodeIds[index];
                    }
                }
            }
        }

        _lastPickInfo = info;
        _hasLastPickInfo = info.kind != PrimitivePickKind::None;
        _lastPickedMesh =
            _hasLastPickInfo && registration != nullptr ? registration->mesh.get() : nullptr;
        if (_pickInfoCallback)
        {
            _pickInfoCallback(info);
        }
    }

    const MeshSelector::MeshRegistration* MeshSelector::findRegistration(
        const Common::SharedPtr<RenderPrimitive>& primitive) const
    {
        for (size_t index = 0; index < _registrations.size(); ++index)
        {
            const auto& registration = _registrations[index];
            if (registration.mesh && (primitive == registration.nodePrimitive ||
                                      primitive == registration.linePrimitive ||
                                      primitive == registration.facePrimitive))
            {
                return &registration;
            }
        }
        return nullptr;
    }

    MeshSelector::~MeshSelector()
    {
    }
} // namespace EgLab::RE