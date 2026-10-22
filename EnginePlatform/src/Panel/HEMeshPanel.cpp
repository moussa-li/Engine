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
        _selectedHalfEdgeIndex = ME::INVALID_IDX;

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
                        _selectedHalfEdgeIndex = ME::INVALID_IDX;
                        RenderWork::instance().setHEMeshHighlight(nullptr, ME::INVALID_IDX);
                    }
                    else
                    {
                        _status = "Node highlighted.";
                        _selectedHalfEdgeIndex = ME::INVALID_IDX;
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
                        _selectedHalfEdgeIndex = ME::INVALID_IDX;
                        RenderWork::instance().setHEMeshHighlight(nullptr, ME::INVALID_IDX);
                    }
                    else
                    {
                        const auto vertex = _halfEdgeMesh->getVertex(_selectedVertexIndex);
                        if (vertex._halfEdge == ME::INVALID_IDX)
                        {
                            _status = "Node has no incident half-edge.";
                            _selectedHalfEdgeIndex = ME::INVALID_IDX;
                            RenderWork::instance().setHEMeshHighlight(_halfEdgeMesh,
                                                                      _selectedVertexIndex);
                        }
                        else
                        {
                            _selectedHalfEdgeIndex = vertex._halfEdge;
                            _status = "Node and outgoing half-edge highlighted.";
                            RenderWork::instance().setHEMeshHighlight(
                                _halfEdgeMesh, _selectedVertexIndex, vertex._halfEdge);
                        }
                    }
                }

                ImGui::Separator();
                const auto navigateEdge = [this](ME::IdxType ME::HalfEdge::* adjacentEdge,
                                                 const char* successStatus,
                                                 const char* missingStatus) {
                    if (_selectedHalfEdgeIndex == ME::INVALID_IDX ||
                        _selectedHalfEdgeIndex >= _halfEdgeMesh->getHalfEdgeCount())
                    {
                        _status = "Select an edge first.";
                        return;
                    }

                    const auto currentEdge = _halfEdgeMesh->getHalfEdge(_selectedHalfEdgeIndex);
                    const auto adjacentEdgeIndex = currentEdge.*adjacentEdge;
                    if (adjacentEdgeIndex == ME::INVALID_IDX ||
                        adjacentEdgeIndex >= _halfEdgeMesh->getHalfEdgeCount())
                    {
                        _status = missingStatus;
                        return;
                    }

                    const auto nextEdge = _halfEdgeMesh->getHalfEdge(adjacentEdgeIndex);
                    if (nextEdge.vertex >= _halfEdgeMesh->getVertexCount())
                    {
                        _status = missingStatus;
                        return;
                    }
                    _selectedHalfEdgeIndex = adjacentEdgeIndex;
                    _selectedVertexIndex = nextEdge.vertex;
                    _status = successStatus;
                    RenderWork::instance().setHEMeshHighlight(_halfEdgeMesh, _selectedVertexIndex,
                                                              _selectedHalfEdgeIndex);
                };

                if (ImGui::Button("nextEdge"))
                {
                    navigateEdge(&ME::HalfEdge::next, "Next edge highlighted.",
                                 "Next edge was not found.");
                }
                ImGui::SameLine();
                if (ImGui::Button("prevEdge"))
                {
                    navigateEdge(&ME::HalfEdge::prev, "Previous edge highlighted.",
                                 "Previous edge was not found.");
                }
                ImGui::SameLine();
                if (ImGui::Button("twinEdge"))
                {
                    navigateEdge(&ME::HalfEdge::twin, "Twin edge highlighted.",
                                 "Twin edge was not found.");
                }
            }
            ImGui::TextWrapped("%s", _status.c_str());
        }
        ImGui::End();
        if (!_show)
        {
            _selectedHalfEdgeIndex = ME::INVALID_IDX;
            RenderWork::instance().setHEMeshHighlight(nullptr, ME::INVALID_IDX);
        }
    }

    void HEMeshPanel::abortClose()
    {
        _selectedHalfEdgeIndex = ME::INVALID_IDX;
        RenderWork::instance().setHEMeshHighlight(nullptr, ME::INVALID_IDX);
    }
} // namespace EgLab::Platform