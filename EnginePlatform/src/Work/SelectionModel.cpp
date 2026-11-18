#include "Work/SelectionModel.hpp"

#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab::Platform
{
    namespace
    {
        void appendIds(const Common::HashSet<ME::IdType>& ids,
                       Common::DynamicArray<uint32_t>& out)
        {
            out.reserve(ids.size());
            for (auto it = ids.begin(); it.hasNext(); it.next())
            {
                out.pushBack(static_cast<uint32_t>(it.data()));
            }
        }
    } // namespace

    SelectionModel::SelectionModel() = default;

    SelectionModel::~SelectionModel() = default;

    void SelectionModel::clear()
    {
        if (_meshStates.empty())
        {
            return;
        }

        _meshStates.clear();
        bumpRevision();
    }

    void SelectionModel::addMeshSelection(const Common::SharedPtr<ME::Mesh>& mesh,
                                          const Common::DynamicArray<uint32_t>& nodeIds,
                                          const Common::DynamicArray<uint32_t>& elementIds)
    {
        if (!mesh || (nodeIds.empty() && elementIds.empty()))
        {
            return;
        }

        SelectionMeshState* state = findMeshState(mesh);
        if (state == nullptr)
        {
            SelectionMeshState newState;
            newState.mesh = mesh;
            _meshStates.pushBack(Common::move(newState));
            state = &_meshStates.back();
        }

        for (size_t index = 0; index < nodeIds.size(); ++index)
        {
            state->nodeIds.insert(static_cast<ME::IdType>(nodeIds[index]));
        }
        for (size_t index = 0; index < elementIds.size(); ++index)
        {
            state->elementIds.insert(static_cast<ME::IdType>(elementIds[index]));
        }

        refreshDerivedState(*state);
        bumpRevision();
    }

    void SelectionModel::removeMesh(const Common::SharedPtr<ME::Mesh>& mesh)
    {
        if (!mesh)
        {
            return;
        }

        Common::DynamicArray<SelectionMeshState> remaining;
        bool removed = false;
        for (auto& state : _meshStates)
        {
            if (state.mesh == mesh)
            {
                removed = true;
            }
            else
            {
                remaining.pushBack(Common::move(state));
            }
        }
        if (!removed)
        {
            return;
        }

        _meshStates = Common::move(remaining);
        bumpRevision();
    }

    void SelectionModel::pruneMesh(const Common::SharedPtr<ME::Mesh>& mesh)
    {
        SelectionMeshState* state = findMeshState(mesh);
        if (state == nullptr)
        {
            return;
        }

        Common::HashSet<ME::IdType> activeNodes;
        ME::MeshIterator nodeIterator(*mesh);
        for (; nodeIterator.hasMoreNode(); nodeIterator.nextNode())
        {
            activeNodes.insert(nodeIterator.currentNode().getId());
        }

        Common::HashSet<ME::IdType> activeElements;
        ME::MeshIterator elemIterator(*mesh);
        for (; elemIterator.hasMoreElem(); elemIterator.nextElem())
        {
            activeElements.insert(elemIterator.currentElem().getId());
        }

        Common::HashSet<ME::IdType> nodes;
        for (auto it = state->nodeIds.begin(); it.hasNext(); it.next())
        {
            if (activeNodes.find(it.data()) != activeNodes.end())
            {
                nodes.insert(it.data());
            }
        }

        Common::HashSet<ME::IdType> elements;
        for (auto it = state->elementIds.begin(); it.hasNext(); it.next())
        {
            if (activeElements.find(it.data()) != activeElements.end())
            {
                elements.insert(it.data());
            }
        }

        state->nodeIds = nodes;
        state->elementIds = elements;
        refreshDerivedState(*state);
        bumpRevision();
    }

    void SelectionModel::getSnapshot(SelectionSnapshot& snapshot) const
    {
        snapshot.meshes.clear();
        snapshot.totalNodes = 0;
        snapshot.totalElements = 0;
        snapshot.revision = _revision;

        for (const auto& state : _meshStates)
        {
            if (!state.mesh)
            {
                continue;
            }

            SelectionMeshSnapshot meshSnapshot;
            meshSnapshot.mesh = state.mesh;
            appendIds(state.nodeIds, meshSnapshot.nodeIds);
            appendIds(state.elementIds, meshSnapshot.elementIds);
            appendIds(state.associatedElementIds, meshSnapshot.associatedElementIds);
            appendIds(state.isolatedNodeIds, meshSnapshot.isolatedNodeIds);

            snapshot.totalNodes += meshSnapshot.nodeIds.size();
            snapshot.totalElements += meshSnapshot.elementIds.size();
            snapshot.meshes.pushBack(Common::move(meshSnapshot));
        }
    }

    bool SelectionModel::empty() const
    {
        for (const auto& state : _meshStates)
        {
            if (!state.nodeIds.empty() || !state.elementIds.empty())
            {
                return false;
            }
        }
        return true;
    }

    SelectionMeshState* SelectionModel::findMeshState(
        const Common::SharedPtr<ME::Mesh>& mesh)
    {
        if (!mesh)
        {
            return nullptr;
        }

        for (auto& state : _meshStates)
        {
            if (state.mesh == mesh)
            {
                return &state;
            }
        }
        return nullptr;
    }

    const SelectionMeshState* SelectionModel::findMeshState(
        const Common::SharedPtr<ME::Mesh>& mesh) const
    {
        if (!mesh)
        {
            return nullptr;
        }

        for (const auto& state : _meshStates)
        {
            if (state.mesh == mesh)
            {
                return &state;
            }
        }
        return nullptr;
    }

    void SelectionModel::refreshDerivedState(SelectionMeshState& state)
    {
        state.associatedElementIds.clear();
        state.isolatedNodeIds.clear();

        if (!state.mesh || state.nodeIds.empty())
        {
            return;
        }

        Common::HashSet<ME::IdType> referencedSelectedNodes;
        ME::MeshIterator elemIterator(*state.mesh);
        for (; elemIterator.hasMoreElem(); elemIterator.nextElem())
        {
            const auto& elem = elemIterator.currentElem();
            bool referencesSelection = false;
            const auto& elemNodes = elem.getNodes();
            for (size_t index = 0; index < elemNodes.size(); ++index)
            {
                if (state.nodeIds.find(elemNodes[index]) != state.nodeIds.end())
                {
                    referencesSelection = true;
                    break;
                }
            }
            if (!referencesSelection)
            {
                continue;
            }

            state.associatedElementIds.insert(elem.getId());
            for (size_t index = 0; index < elemNodes.size(); ++index)
            {
                if (state.nodeIds.find(elemNodes[index]) != state.nodeIds.end())
                {
                    referencedSelectedNodes.insert(elemNodes[index]);
                }
            }
        }

        for (auto it = state.nodeIds.begin(); it.hasNext(); it.next())
        {
            if (referencedSelectedNodes.find(it.data()) == referencedSelectedNodes.end())
            {
                state.isolatedNodeIds.insert(it.data());
            }
        }
    }

    void SelectionModel::bumpRevision()
    {
        ++_revision;
    }
} // namespace EgLab::Platform
