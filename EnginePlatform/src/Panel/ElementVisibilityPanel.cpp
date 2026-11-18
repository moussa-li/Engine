#include "Panel/ElementVisibilityPanel.hpp"

#include <cmath>
#include <limits>

#include "Command/CommandManager.hpp"
#include "Common/Log.hpp"
#include "Common/Vector.hpp"
#include "DataBase/Context.hpp"
#include "DataBase/MeshData.hpp"
#include "MeshEngine/Algorithm/PlaneCut.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "Work/RenderWork.hpp"
#include "Work/UIWork.hpp"
#include "imgui.h"

namespace
{
    // Translucent blue used for the cut plane preview so the mesh behind it stays visible.
    EgLab::Common::Vector4f previewColor(float opacity)
    {
        const float alpha = opacity < 0.05f ? 0.05f : (opacity > 1.0f ? 1.0f : opacity);
        return EgLab::Common::Vector4f(0.2f, 0.6f, 1.0f, alpha);
    }

    // Builds the quad mesh that visualises the plane. The quad lies inside the plane, so its
    // two in-plane axes are derived from the normal.
    EgLab::Common::SharedPtr<EgLab::ME::Mesh> buildPlaneMesh(
        const EgLab::Common::Vector3f& normal,
        const EgLab::Common::Vector3f& referencePoint, float halfExtent)
    {
        EgLab::Common::Vector3f unitNormal = normal;
        unitNormal.normalize();
        if (unitNormal.length() < 0.5f || halfExtent <= 0.0f)
        {
            return EgLab::Common::SharedPtr<EgLab::ME::Mesh>(nullptr);
        }

        // Pick a helper axis that is not parallel to the normal so the cross product stays
        // well defined for every orientation.
        EgLab::Common::Vector3f helper(0.0f, 0.0f, 1.0f);
        if (std::abs(unitNormal.z()) > 0.9f)
        {
            helper = EgLab::Common::Vector3f(0.0f, 1.0f, 0.0f);
        }
        EgLab::Common::Vector3f axisU = helper.cross(unitNormal);
        axisU.normalize();
        const EgLab::Common::Vector3f axisV = unitNormal.cross(axisU);

        const EgLab::Common::Vector3f corners[4] = {
            referencePoint + (axisU * -1.0f + axisV * -1.0f) * halfExtent,
            referencePoint + (axisU + axisV * -1.0f) * halfExtent,
            referencePoint + (axisU + axisV) * halfExtent,
            referencePoint + (axisU * -1.0f + axisV) * halfExtent,
        };

        auto mesh = EgLab::Common::makeShared<EgLab::ME::Mesh>();
        for (uint32_t index = 0; index < 4; ++index)
        {
            EgLab::ME::Node node;
            node.setId(index + 1);
            EgLab::ME::CoordType xyz(corners[index].x(), corners[index].y(), corners[index].z());
            node.setXYZ(xyz);
            mesh->addNode(EgLab::Common::move(node));
        }

        EgLab::ME::Elem elem;
        elem.setId(1);
        elem.setType(EgLab::ME::ElemType::Quad4);
        for (uint32_t index = 0; index < 4; ++index)
        {
            elem.setNode(index, index + 1);
        }
        mesh->addElem(EgLab::Common::move(elem));
        return mesh;
    }

    // Formats an element id list as "1, 2, 3" without repeated string concatenation so that
    // large plane-cut id sets are cheap to build.
    EgLab::Common::String formatElementIds(const EgLab::Common::DynamicArray<uint32_t>& ids)
    {
        if (ids.empty())
        {
            return EgLab::Common::String();
        }

        EgLab::Common::DynamicArray<char> buffer;
        buffer.reserve(ids.size() * 8);

        char digits[16];
        for (size_t index = 0; index < ids.size(); ++index)
        {
            if (index != 0)
            {
                buffer.pushBack(',');
                buffer.pushBack(' ');
            }

            uint32_t value = ids[index];
            char* cursor = digits + sizeof(digits) - 1;
            *cursor = '\0';
            do
            {
                --cursor;
                *cursor = static_cast<char>('0' + value % 10);
                value /= 10;
            } while (value != 0);

            for (const char* digit = cursor; *digit != '\0'; ++digit)
            {
                buffer.pushBack(*digit);
            }
        }

        return EgLab::Common::String(buffer.data(), buffer.size());
    }
} // namespace

namespace EgLab::Platform
{
    ElementVisibilityPanel::ElementVisibilityPanel()
    {
    }

    ElementVisibilityPanel::~ElementVisibilityPanel()
    {
        if (_selectionInfoSubscriptionId != 0)
        {
            RenderWork::instance().unsubscribeSelectionChanged(_selectionInfoSubscriptionId);
        }
    }

    void ElementVisibilityPanel::configureWindow() const
    {
        ImGui::SetNextWindowSize(ImVec2(400, 240), ImGuiCond_FirstUseEver);
    }

    void ElementVisibilityPanel::refreshMeshEntries()
    {
        const DataNodeId previousId =
            (_selectedMeshIndex >= 0 && _selectedMeshIndex < static_cast<int>(_meshEntries.size()))
                ? _meshEntries[_selectedMeshIndex].nodeId
                : 0;
        const DataNodeId previousScopeId =
            (_selectionScopeIndex > 0 &&
             _selectionScopeIndex <= static_cast<int>(_meshEntries.size()))
                ? _meshEntries[_selectionScopeIndex - 1].nodeId
                : 0;

        _meshEntries.clear();

        auto& dataTree = Context::instance().getDataTree();
        auto collect = [&](auto&& self,
                           const Common::DynamicArray<Common::SharedPtr<TreeNode>>& nodes) -> void {
            for (const auto& node : nodes)
            {
                if (auto* meshData = dynamic_cast<MeshData*>(node.get()))
                {
                    MeshEntry entry;
                    entry.nodeId = meshData->getId();
                    entry.name = meshData->getName();
                    _meshEntries.pushBack(entry);
                }
                self(self, node->getChildren());
            }
        };
        collect(collect, dataTree.getChildren());

        _selectedMeshIndex = _meshEntries.empty() ? -1 : 0;
        if (previousId != 0)
        {
            for (int index = 0; index < static_cast<int>(_meshEntries.size()); ++index)
            {
                if (_meshEntries[index].nodeId == previousId)
                {
                    _selectedMeshIndex = index;
                    break;
                }
            }
        }

        _selectionScopeIndex = 0;
        if (previousScopeId != 0)
        {
            for (int index = 0; index < static_cast<int>(_meshEntries.size()); ++index)
            {
                if (_meshEntries[index].nodeId == previousScopeId)
                {
                    _selectionScopeIndex = index + 1;
                    break;
                }
            }
        }

        // Keep the render-side scope in sync after mesh additions/removals.
        RenderWork::instance().setSelectionScope(getMeshByEntryIndex(_selectionScopeIndex));
    }

    Common::SharedPtr<ME::Mesh> ElementVisibilityPanel::getMeshByEntryIndex(int entryIndex) const
    {
        if (entryIndex <= 0 || entryIndex > static_cast<int>(_meshEntries.size()))
        {
            return nullptr;
        }

        auto node = Context::instance().getDataNode(_meshEntries[entryIndex - 1].nodeId);
        auto* meshData = dynamic_cast<MeshData*>(node.get());
        return meshData ? meshData->getMesh() : nullptr;
    }

    bool ElementVisibilityPanel::findMeshEntry(const Common::SharedPtr<ME::Mesh>& mesh,
                                               MeshEntry& entry) const
    {
        if (!mesh)
        {
            return false;
        }

        for (const auto& candidate : _meshEntries)
        {
            auto node = Context::instance().getDataNode(candidate.nodeId);
            auto* meshData = dynamic_cast<MeshData*>(node.get());
            if (meshData && meshData->getMesh() == mesh)
            {
                entry = candidate;
                return true;
            }
        }
        return false;
    }

    Common::SharedPtr<ME::Mesh> ElementVisibilityPanel::getSelectedMesh() const
    {
        if (_selectedMeshIndex < 0 ||
            _selectedMeshIndex >= static_cast<int>(_meshEntries.size()))
        {
            return nullptr;
        }

        auto node = Context::instance().getDataNode(_meshEntries[_selectedMeshIndex].nodeId);
        auto* meshData = dynamic_cast<MeshData*>(node.get());
        return meshData ? meshData->getMesh() : nullptr;
    }

    void ElementVisibilityPanel::updateReferenceBounds(const Common::SharedPtr<ME::Mesh>& mesh,
                                                       DataNodeId nodeId)
    {
        _boundsNodeId = nodeId;
        const float floatMax = std::numeric_limits<float>::max();
        const float floatLowest = std::numeric_limits<float>::lowest();
        for (int axis = 0; axis < 3; ++axis)
        {
            _boundsMin[axis] = floatMax;
            _boundsMax[axis] = floatLowest;
        }

        ME::MeshIterator iterator(*mesh);
        bool hasNode = false;
        for (; iterator.hasMoreNode(); iterator.nextNode())
        {
            const auto& node = iterator.currentNode();
            hasNode = true;
            const float coordinates[3] = {
                static_cast<float>(node.x()), static_cast<float>(node.y()),
                static_cast<float>(node.z())};
            for (int axis = 0; axis < 3; ++axis)
            {
                if (coordinates[axis] < _boundsMin[axis]) _boundsMin[axis] = coordinates[axis];
                if (coordinates[axis] > _boundsMax[axis]) _boundsMax[axis] = coordinates[axis];
            }
        }

        if (!hasNode)
        {
            for (int axis = 0; axis < 3; ++axis)
            {
                _boundsMin[axis] = -10.0f;
                _boundsMax[axis] = 10.0f;
            }
        }

        _referenceMin = std::min(_boundsMin[0], std::min(_boundsMin[1], _boundsMin[2]));
        _referenceMax = std::max(_boundsMax[0], std::max(_boundsMax[1], _boundsMax[2]));
        if (_referenceMax - _referenceMin < 1e-6f)
        {
            _referenceMin -= 1.0f;
            _referenceMax += 1.0f;
        }

        // Size the preview quad from the bounding box diagonal so it always covers the mesh
        // regardless of which way the plane is oriented.
        const float extentX = _boundsMax[0] - _boundsMin[0];
        const float extentY = _boundsMax[1] - _boundsMin[1];
        const float extentZ = _boundsMax[2] - _boundsMin[2];
        const float diagonal = std::sqrt(extentX * extentX + extentY * extentY + extentZ * extentZ);
        _previewHalfExtent = diagonal * 0.55f;
        if (_previewHalfExtent < 1.0f)
        {
            _previewHalfExtent = 1.0f;
        }

        for (int axis = 0; axis < 3; ++axis)
        {
            if (_referencePoint[axis] < _boundsMin[axis])
            {
                _referencePoint[axis] = _boundsMin[axis];
            }
            if (_referencePoint[axis] > _boundsMax[axis])
            {
                _referencePoint[axis] = _boundsMax[axis];
            }
        }
    }

    void ElementVisibilityPanel::applyPlaneCut()
    {
        auto mesh = getSelectedMesh();
        if (!mesh)
        {
            return;
        }

        const ME::CoordType normal(static_cast<double>(_normal[0]),
                                   static_cast<double>(_normal[1]),
                                   static_cast<double>(_normal[2]));
        if (normal.length() < 1e-12)
        {
            LOG(WARNING) << "ElementVisibilityPanel::applyPlaneCut() plane normal is zero";
            return;
        }

        const ME::CoordType point(static_cast<double>(_referencePoint[0]),
                                  static_cast<double>(_referencePoint[1]),
                                  static_cast<double>(_referencePoint[2]));

        ME::PlaneCut planeCut(mesh);
        const ME::PlaneCutResult result = planeCut.cut(normal, point);
        _positiveElementIds = result.positiveElementIds;
        _negativeElementIds = result.negativeElementIds;

        const DataNodeId nodeId = _meshEntries[_selectedMeshIndex].nodeId;
        // Show positive and hide negative in a single command call. Describe the result with
        // the smaller id list: mode 3 hides only the negative side, mode 2 shows only the
        // positive side.
        if (_negativeElementIds.size() <= _positiveElementIds.size())
        {
            requestElementsVisibility(nodeId, 3, formatElementIds(_negativeElementIds));
        }
        else
        {
            requestElementsVisibility(nodeId, 2, formatElementIds(_positiveElementIds));
        }
    }

    void ElementVisibilityPanel::requestShowAll()
    {
        if (_selectedMeshIndex < 0 ||
            _selectedMeshIndex >= static_cast<int>(_meshEntries.size()))
        {
            return;
        }
        requestElementsVisibility(_meshEntries[_selectedMeshIndex].nodeId, 1, Common::String());
    }

    void ElementVisibilityPanel::requestElementsVisibility(DataNodeId nodeId, uint32_t mode,
                                                           const Common::String& ids)
    {
        EventPacket packet;
        packet.setData("nodeId", nodeId);
        packet.setData("mode", mode);
        packet.setData("ids", ids);
        CommandManager::instance().call(EventId::ElementsVisibility, Common::move(packet));
    }

    void ElementVisibilityPanel::requestNodeVisibility(DataNodeId nodeId, uint32_t mode,
                                                       const Common::String& ids)
    {
        EventPacket packet;
        packet.setData("nodeId", nodeId);
        packet.setData("mode", mode);
        packet.setData("ids", ids);
        CommandManager::instance().call(EventId::NodeVisibility, Common::move(packet));
    }

    void ElementVisibilityPanel::refreshSelectionSnapshot()
    {
        SelectionChangedInfo info;
        while (RenderWork::instance().tryReceiveSelectionChanged(_selectionInfoSubscriptionId,
                                                                 info))
        {
            _selectionCounts = info;
            _selectionSnapshotDirty = true;
        }

        if (_selectionSnapshotDirty || _selectionSnapshot.revision == 0)
        {
            RenderWork::instance().copySelectionSnapshot(_selectionSnapshot);
            _selectionCounts.nodeCount = _selectionSnapshot.totalNodes;
            _selectionCounts.elementCount = _selectionSnapshot.totalElements;
            _selectionCounts.revision = _selectionSnapshot.revision;
            _selectionSnapshotDirty = false;
        }
    }

    void ElementVisibilityPanel::applySelectionVisibility(uint32_t elementMode,
                                                          uint32_t nodeMode)
    {
        for (const auto& meshSelection : _selectionSnapshot.meshes)
        {
            MeshEntry entry;
            if (!findMeshEntry(meshSelection.mesh, entry))
            {
                continue;
            }

            Common::DynamicArray<uint32_t> elementIds;
            elementIds.reserve(meshSelection.elementIds.size() +
                               meshSelection.associatedElementIds.size());
            for (size_t index = 0; index < meshSelection.elementIds.size(); ++index)
            {
                elementIds.pushBack(meshSelection.elementIds[index]);
            }
            for (size_t index = 0; index < meshSelection.associatedElementIds.size(); ++index)
            {
                elementIds.pushBack(meshSelection.associatedElementIds[index]);
            }

            // Isolate must also hide every non-selected element, which is expressed by a
            // mode-2 request with an empty id list.
            if (!elementIds.empty() || elementMode == 2)
            {
                requestElementsVisibility(entry.nodeId, elementMode, formatElementIds(elementIds));
            }

            // Node hiding only applies to nodes no element references; referenced nodes are
            // hidden together with their elements, so the node channel is skipped for them.
            const auto& nodeIds =
                nodeMode == 2 ? meshSelection.nodeIds : meshSelection.isolatedNodeIds;
            if (!nodeIds.empty())
            {
                requestNodeVisibility(entry.nodeId, nodeMode, formatElementIds(nodeIds));
            }
        }
    }

    void ElementVisibilityPanel::requestShowAllInScope()
    {
        const auto showAllForEntry = [this](const MeshEntry& entry) {
            requestElementsVisibility(entry.nodeId, 1, Common::String());
            requestNodeVisibility(entry.nodeId, 1, Common::String());
        };

        if (_selectionScopeIndex > 0 &&
            _selectionScopeIndex <= static_cast<int>(_meshEntries.size()))
        {
            showAllForEntry(_meshEntries[_selectionScopeIndex - 1]);
            return;
        }

        for (const auto& entry : _meshEntries)
        {
            showAllForEntry(entry);
        }
    }

    void ElementVisibilityPanel::renderSelectionSection()
    {
        ImGui::SeparatorText("Selection");

        bool boxSelectionEnabled = UIWork::instance().isBoxSelectionEnabled();
        if (ImGui::Checkbox("Box Select Mode", &boxSelectionEnabled))
        {
            UIWork::instance().setBoxSelectionEnabled(boxSelectionEnabled);
        }
        ImGui::SameLine();
        ImGui::TextDisabled("LMB drag in viewport; Ctrl appends; Esc clears");

        const char* scopeLabel = "All Visible Meshes";
        if (_selectionScopeIndex > 0 &&
            _selectionScopeIndex <= static_cast<int>(_meshEntries.size()))
        {
            scopeLabel = _meshEntries[_selectionScopeIndex - 1].name.c_str();
        }
        if (ImGui::BeginCombo("Selection Range", scopeLabel))
        {
            if (ImGui::Selectable("All Visible Meshes", _selectionScopeIndex == 0))
            {
                _selectionScopeIndex = 0;
                RenderWork::instance().setSelectionScope(nullptr);
            }
            for (int index = 0; index < static_cast<int>(_meshEntries.size()); ++index)
            {
                const bool selected = _selectionScopeIndex == index + 1;
                if (ImGui::Selectable(_meshEntries[index].name.c_str(), selected))
                {
                    _selectionScopeIndex = index + 1;
                    RenderWork::instance().setSelectionScope(getMeshByEntryIndex(index + 1));
                }
                if (selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::Checkbox("Contains Only", &_selectionContainsOnly))
        {
            RenderWork::instance().setSelectionContainsOnly(_selectionContainsOnly);
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(unchecked = intersect)");

        ImGui::Text("Selected nodes: %llu", static_cast<unsigned long long>(_selectionCounts.nodeCount));
        ImGui::Text("Selected elements: %llu",
                    static_cast<unsigned long long>(_selectionCounts.elementCount));

        const bool hasSelection = _selectionCounts.nodeCount > 0 || _selectionCounts.elementCount > 0;
        ImGui::BeginDisabled(!hasSelection);
        if (ImGui::Button("Hide Selected"))
        {
            applySelectionVisibility(0, 0);
        }
        ImGui::SameLine();
        if (ImGui::Button("Show Selected"))
        {
            applySelectionVisibility(1, 1);
        }
        ImGui::SameLine();
        if (ImGui::Button("Isolate Selected"))
        {
            applySelectionVisibility(2, 2);
        }
        ImGui::EndDisabled();

        if (ImGui::Button("Show All"))
        {
            requestShowAllInScope();
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(!hasSelection);
        if (ImGui::Button("Clear Selection"))
        {
            RenderWork::instance().requestClearSelection();
            _selectionSnapshot.meshes.clear();
            _selectionSnapshot.totalNodes = 0;
            _selectionSnapshot.totalElements = 0;
            _selectionCounts.nodeCount = 0;
            _selectionCounts.elementCount = 0;
        }
        ImGui::EndDisabled();
    }

    void ElementVisibilityPanel::updatePlanePreview()
    {
        hidePlanePreview();

        if (!_showPreview)
        {
            return;
        }

        const Common::Vector3f normal(_normal[0], _normal[1], _normal[2]);
        const Common::Vector3f referencePoint(_referencePoint[0], _referencePoint[1],
                                              _referencePoint[2]);
        auto mesh = buildPlaneMesh(normal, referencePoint, _previewHalfExtent);
        if (!mesh)
        {
            LOG(WARNING) << "ElementVisibilityPanel::updatePlanePreview() skipped an invalid "
                            "plane";
            return;
        }

        _previewMesh = mesh;
        // Record the colour before the geometry so the faces are uploaded already translucent.
        RenderWork::instance().setMeshColor(mesh, previewColor(_previewOpacity));
        RenderWork::instance().onUpdateMesh(mesh);
    }

    void ElementVisibilityPanel::hidePlanePreview()
    {
        if (!_previewMesh)
        {
            return;
        }

        RenderWork::instance().removeMesh(_previewMesh);
        _previewMesh = nullptr;
    }

    void ElementVisibilityPanel::afterRenderFrame()
    {
        if (!isShow())
        {
            hidePlanePreview();
            RenderWork::instance().setSelectionScope(nullptr);
        }
    }

    void ElementVisibilityPanel::abortShow()
    {
        // The preview is hidden when the panel closes, so rebuild it when the panel returns.
        if (_showPreview && _boundsNodeId != 0)
        {
            updatePlanePreview();
        }
    }

    void ElementVisibilityPanel::abortClose()
    {
        hidePlanePreview();
        RenderWork::instance().setSelectionScope(nullptr);
    }

    void ElementVisibilityPanel::renderImpl()
    {
        refreshMeshEntries();

        if (_selectionInfoSubscriptionId == 0)
        {
            _selectionInfoSubscriptionId = RenderWork::instance().subscribeSelectionChanged();
        }
        RenderWork::instance().setSelectionContainsOnly(_selectionContainsOnly);
        refreshSelectionSnapshot();

        if (_meshEntries.empty())
        {
            hidePlanePreview();
            ImGui::TextDisabled("No mesh data nodes available.");
            return;
        }

        if (_selectedMeshIndex < 0 ||
            _selectedMeshIndex >= static_cast<int>(_meshEntries.size()))
        {
            _selectedMeshIndex = 0;
        }

        if (ImGui::BeginCombo("Mesh", _meshEntries[_selectedMeshIndex].name.c_str()))
        {
            for (int index = 0; index < static_cast<int>(_meshEntries.size()); ++index)
            {
                const bool selected = index == _selectedMeshIndex;
                if (ImGui::Selectable(_meshEntries[index].name.c_str(), selected))
                {
                    _selectedMeshIndex = index;
                }
                if (selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        renderSelectionSection();

        auto mesh = getSelectedMesh();
        if (!mesh)
        {
            hidePlanePreview();
            ImGui::TextDisabled("Selected mesh has no geometry.");
            return;
        }

        const DataNodeId selectedNodeId = _meshEntries[_selectedMeshIndex].nodeId;
        bool previewChanged = false;
        if (_boundsNodeId != selectedNodeId)
        {
            _positiveElementIds.clear();
            _negativeElementIds.clear();
            updateReferenceBounds(mesh, selectedNodeId);
            previewChanged = true;
        }

        if (ImGui::SliderFloat3("Plane Normal", _normal, -1.0f, 1.0f))
        {
            previewChanged = true;
        }
        if (_autoApply && ImGui::IsItemDeactivatedAfterEdit())
        {
            applyPlaneCut();
        }

        if (ImGui::SliderFloat3("Reference Point", _referencePoint, _referenceMin, _referenceMax))
        {
            previewChanged = true;
        }
        if (_autoApply && ImGui::IsItemDeactivatedAfterEdit())
        {
            applyPlaneCut();
        }

        ImGui::Checkbox("Auto Apply", &_autoApply);

        if (ImGui::Checkbox("Show Preview Plane", &_showPreview))
        {
            previewChanged = true;
        }
        if (_showPreview)
        {
            if (ImGui::SliderFloat("Preview Opacity", &_previewOpacity, 0.05f, 1.0f, "%.2f"))
            {
                previewChanged = true;
            }
        }

        if (previewChanged)
        {
            updatePlanePreview();
        }

        ImGui::Separator();

        if (ImGui::Button("Apply Plane Cut"))
        {
            applyPlaneCut();
        }
        ImGui::SameLine();
        if (ImGui::Button("Show All"))
        {
            requestShowAll();
        }

        ImGui::Text("Positive side: %zu", _positiveElementIds.size());
        ImGui::Text("Negative side: %zu", _negativeElementIds.size());
    }
} // namespace EgLab::Platform
