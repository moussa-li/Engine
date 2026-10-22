#pragma once

#include "Common/SharedPtr.hpp"
#include "Common/String.hpp"
#include "MeshEngine/MeshData/HEMesh.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "Panel/IPanel.hpp"

namespace EgLab::Platform
{
    class HEMeshPanel : public IPanel
    {
    public:
        Common::String getPanelName() override
        {
            return "HEMeshInspector";
        }

        void setMesh(const Common::SharedPtr<ME::Mesh>& mesh, const Common::String& meshName);
        void render() override;

    protected:
        void abortClose() override;

    private:
        Common::SharedPtr<ME::Mesh> _sourceMesh;
        Common::SharedPtr<ME::HEMesh> _halfEdgeMesh;
        Common::String _meshName;
        Common::String _status{"Select a mesh from the Data Tree."};
        ME::IdType _nodeId{0};
        ME::IdxType _selectedVertexIndex{ME::INVALID_IDX};
        ME::IdxType _selectedHalfEdgeIndex{ME::INVALID_IDX};
    };
} // namespace EgLab::Platform