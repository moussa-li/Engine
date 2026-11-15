#include "Panel/ElementVisibilityPanel.hpp"

#include <cmath>
#include <limits>

#include "Command/CommandManager.hpp"
#include "Common/Log.hpp"
#include "DataBase/Context.hpp"
#include "DataBase/MeshData.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "Work/RenderWork.hpp"
#include "imgui.h"

namespace
{
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

        const float spanX = _boundsMax[0] - _boundsMin[0];
        const float spanY = _boundsMax[1] - _boundsMin[1];
        const float spanZ = _boundsMax[2] - _boundsMin[2];
        _planeExtent = std::sqrt(spanX * spanX + spanY * spanY + spanZ * spanZ);
        if (_planeExtent < 1e-6f)
        {
            _planeExtent = 10.0f;
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

        float normalX = _normal[0];
        float normalY = _normal[1];
        float normalZ = _normal[2];
        const float normalLength = std::sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ);
        if (normalLength < 1e-6f)
        {
            LOG(WARNING) << "ElementVisibilityPanel::applyPlaneCut() plane normal is zero";
            return;
        }
        normalX /= normalLength;
        normalY /= normalLength;
        normalZ /= normalLength;

        _positiveElementIds.clear();
        _negativeElementIds.clear();
        ME::MeshIterator iterator(*mesh);
        for (; iterator.hasMoreElem(); iterator.nextElem())
        {
            const auto& element = iterator.currentElem();
            const auto& nodeIds = element.getNodes();
            if (nodeIds.empty())
            {
                continue;
            }

            double centerX = 0.0;
            double centerY = 0.0;
            double centerZ = 0.0;
            for (size_t nodeIndex = 0; nodeIndex < nodeIds.size(); ++nodeIndex)
            {
                const auto& node = mesh->getNodeById(nodeIds[nodeIndex]);
                centerX += node.x();
                centerY += node.y();
                centerZ += node.z();
            }
            centerX /= static_cast<double>(nodeIds.size());
            centerY /= static_cast<double>(nodeIds.size());
            centerZ /= static_cast<double>(nodeIds.size());

            const double signedDistance =
                normalX * (centerX - _referencePoint[0]) +
                normalY * (centerY - _referencePoint[1]) +
                normalZ * (centerZ - _referencePoint[2]);
            if (signedDistance >= 0.0)
            {
                _positiveElementIds.pushBack(element.getId());
            }
            else
            {
                _negativeElementIds.pushBack(element.getId());
            }
        }

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

    void ElementVisibilityPanel::abortClose()
    {
        if (_planePreviewVisible)
        {
            RenderWork::instance().setVisibilityPlane(
                Common::Vector3f(_lastNormal[0], _lastNormal[1], _lastNormal[2]),
                Common::Vector3f(_lastPoint[0], _lastPoint[1], _lastPoint[2]),
                _lastExtent, false);
            _planePreviewVisible = false;
        }
    }

    void ElementVisibilityPanel::syncPlanePreview()
    {
        const bool changed =
            _normal[0] != _lastNormal[0] || _normal[1] != _lastNormal[1] ||
            _normal[2] != _lastNormal[2] || _referencePoint[0] != _lastPoint[0] ||
            _referencePoint[1] != _lastPoint[1] || _referencePoint[2] != _lastPoint[2] ||
            _planeExtent != _lastExtent;

        if (_planePreviewVisible && !changed)
        {
            return;
        }

        RenderWork::instance().setVisibilityPlane(
            Common::Vector3f(_normal[0], _normal[1], _normal[2]),
            Common::Vector3f(_referencePoint[0], _referencePoint[1], _referencePoint[2]),
            _planeExtent, true);

        _lastNormal[0] = _normal[0];
        _lastNormal[1] = _normal[1];
        _lastNormal[2] = _normal[2];
        _lastPoint[0] = _referencePoint[0];
        _lastPoint[1] = _referencePoint[1];
        _lastPoint[2] = _referencePoint[2];
        _lastExtent = _planeExtent;
        _planePreviewVisible = true;
    }

    void ElementVisibilityPanel::renderImpl()
    {
        refreshMeshEntries();

        if (_meshEntries.empty())
        {
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

        auto mesh = getSelectedMesh();
        if (!mesh)
        {
            ImGui::TextDisabled("Selected mesh has no geometry.");
            return;
        }

        const DataNodeId selectedNodeId = _meshEntries[_selectedMeshIndex].nodeId;
        if (_boundsNodeId != selectedNodeId)
        {
            _positiveElementIds.clear();
            _negativeElementIds.clear();
            updateReferenceBounds(mesh, selectedNodeId);
        }

        ImGui::SliderFloat3("Plane Normal", _normal, -1.0f, 1.0f);
        if (_autoApply && ImGui::IsItemDeactivatedAfterEdit())
        {
            applyPlaneCut();
        }

        ImGui::SliderFloat3("Reference Point", _referencePoint, _referenceMin, _referenceMax);
        if (_autoApply && ImGui::IsItemDeactivatedAfterEdit())
        {
            applyPlaneCut();
        }

        ImGui::Checkbox("Auto Apply", &_autoApply);
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

        syncPlanePreview();
    }
} // namespace EgLab::Platform
