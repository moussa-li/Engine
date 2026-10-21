#include "Panel/HEMeshPanel.hpp"

#include "MeshEngine/MeshData/MeshToHEMesh.hpp"
#include "Work/RenderWork.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    void HEMeshPanel::setMesh(const Common::SharedPtr<ME::Mesh>& mesh,
                              const Common::String& meshName)
    {
        RenderWork::instance().setHEMeshHighlight(nullptr, ME::INVALID_IDX);
        _sourceMesh = mesh;
        _meshName = meshName;
        _halfEdgeMesh = nullptr;
        _nodeId = 0;
        _selectedVertexIndex = ME::INVALID_IDX;

        if (ME::MeshToHEMesh::convert(_sourceMesh, _halfEdgeMesh) == Common::Return::Succeed)
        {
            _status = "Half-edge mesh created.";
        }
        else
        {
            _status = "Mesh conversion failed. Supported input: Tri3, Quad4, or Tet4.";
        }
    }

    void HEMeshPanel::render()
    {
        ImGui::SetNextWindowSize(ImVec2(360, 260), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Half-Edge Mesh", &_show))
        {
            if (_halfEdgeMesh)
            {
                ImGui::Text("Mesh: %s", _meshName.c_str());
                ImGui::Text("Vertices: %zu  Faces: %zu  Half-edges: %zu",
                            _halfEdgeMesh->getVertexCount(), _halfEdgeMesh->getFaceCount(),
                            _halfEdgeMesh->getHalfEdgeCount());
                ImGui::Separator();
                ImGui::InputScalar("Node ID", ImGuiDataType_U32, &_nodeId);

                if (ImGui::Button("Highlight Node"))
                {
                    _selectedVertexIndex = _halfEdgeMesh->getVertexIndex(_nodeId);
                    if (_selectedVertexIndex == ME::INVALID_IDX)
                    {
                        _status = "Node ID was not found.";
                        RenderWork::instance().setHEMeshHighlight(nullptr, ME::INVALID_IDX);
                    }
                    else
                    {
                        _status = "Node highlighted.";
                        RenderWork::instance().setHEMeshHighlight(_halfEdgeMesh,
                                                                  _selectedVertexIndex);
                    }
                }

                ImGui::SameLine();
                if (ImGui::Button("getEdge"))
                {
                    _selectedVertexIndex = _halfEdgeMesh->getVertexIndex(_nodeId);
                    if (_selectedVertexIndex == ME::INVALID_IDX)
                    {
                        _status = "Node ID was not found.";
                        RenderWork::instance().setHEMeshHighlight(nullptr, ME::INVALID_IDX);
                    }
                    else
                    {
                        const auto vertex = _halfEdgeMesh->getVertex(_selectedVertexIndex);
                        if (vertex._halfEdge == ME::INVALID_IDX)
                        {
                            _status = "Node has no incident half-edge.";
                            RenderWork::instance().setHEMeshHighlight(_halfEdgeMesh,
                                                                      _selectedVertexIndex);
                        }
                        else
                        {
                            _status = "Node and outgoing half-edge highlighted.";
                            RenderWork::instance().setHEMeshHighlight(
                                _halfEdgeMesh, _selectedVertexIndex, vertex._halfEdge);
                        }
                    }
                }
            }
            ImGui::TextWrapped("%s", _status.c_str());
        }
        ImGui::End();
        if (!_show)
        {
            RenderWork::instance().setHEMeshHighlight(nullptr, ME::INVALID_IDX);
        }
    }

    void HEMeshPanel::abortClose()
    {
        RenderWork::instance().setHEMeshHighlight(nullptr, ME::INVALID_IDX);
    }
} // namespace EgLab::Platform