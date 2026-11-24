#include "Panel/GeometryInfoPanel.hpp"

#include <sstream>

#include "Command/CommandManager.hpp"
#include "Core/IniManager.hpp"
#include "Work/RenderWork.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    namespace
    {
        constexpr const char* GEOMETRY_PICK_FILTER_KEY = "geometry.pickFilter";
    } // namespace

    GeometryInfoPanel::GeometryInfoPanel()
    {
        const int savedMask = IniManager::instance().getInt(
            GEOMETRY_PICK_FILTER_KEY, static_cast<int>(GEOMETRY_FEATURE_MASK_ALL));
        _pickFilter = static_cast<uint32_t>(savedMask) & GEOMETRY_FEATURE_MASK_ALL;
    }

    GeometryInfoPanel::~GeometryInfoPanel()
    {
        releasePickRequest();
        if (_pickInfoSubscriptionId != 0)
        {
            RenderWork::instance().unsubscribePickInfo(_pickInfoSubscriptionId);
            _pickInfoSubscriptionId = 0;
        }
    }

    Common::String GeometryInfoPanel::featureTypeName(uint32_t type)
    {
        switch (type)
        {
        case 0:
            return Common::String("Vertex");
        case 1:
            return Common::String("Edge");
        case 2:
            return Common::String("Face");
        case 3:
            return Common::String("Solid");
        default:
            return Common::String("Unknown");
        }
    }

    Common::String GeometryInfoPanel::geometryTypeName(uint32_t type)
    {
        switch (type)
        {
        case 0:
            return Common::String("Unknown");
        case 1:
            return Common::String("Point");
        case 2:
            return Common::String("Curve");
        case 3:
            return Common::String("Surface");
        case 4:
            return Common::String("Solid");
        default:
            return Common::String("Unknown");
        }
    }

    void GeometryInfoPanel::syncPickRequest()
    {
        RE::PickManager& picks = RenderWork::instance().pickManager();
        if (isShow() && _showHoverInfo && _facePickToken == RE::PickManager::kInvalidToken)
        {
            _facePickToken = picks.acquire(RE::PickKind::Face, Common::String("GeometryInfoPanel"));
        }
        else if ((!isShow() || !_showHoverInfo) && _facePickToken != RE::PickManager::kInvalidToken)
        {
            picks.release(_facePickToken);
            _facePickToken = RE::PickManager::kInvalidToken;
        }
    }

    void GeometryInfoPanel::releasePickRequest()
    {
        RE::PickManager& picks = RenderWork::instance().pickManager();
        if (_facePickToken != RE::PickManager::kInvalidToken)
        {
            picks.release(_facePickToken);
            _facePickToken = RE::PickManager::kInvalidToken;
        }
    }

    void GeometryInfoPanel::updatePickedInfo()
    {
        _hasPickedInfo = false;
        if (_pickInfoSubscriptionId == 0)
        {
            return;
        }

        PickedMeshInfo pickedInfo;
        while (RenderWork::instance().tryReceivePickInfo(_pickInfoSubscriptionId, pickedInfo))
        {
            _pickedInfo = pickedInfo;
            if (pickedInfo.kind == PickedMeshKind::ElementFace ||
                pickedInfo.kind == PickedMeshKind::ElementEdge)
            {
                auto geometryData = GeometryData::findByMesh(pickedInfo.meshId);
                if (geometryData)
                {
                    // Resolve the same feature the pick selector reported, so a hit on a
                    // filtered-out face shows the solid that stands in for it.
                    const GeometryFeatureRef feature =
                        geometryData->getFeatureByElement(pickedInfo.entityId);
                    const GeometryFeatureRef resolved =
                        geometryData->resolveByMask(feature, _pickFilter);
                    _featureInfo = geometryData->getFeatureInfo(resolved);
                    if (_featureInfo.ref.isValid())
                    {
                        _featureInfo.elemId = pickedInfo.entityId;
                        _hasPickedInfo = true;
                    }
                    else
                    {
                        _hoverStatus = Common::String("picked face has no geometry feature");
                    }
                }
                else
                {
                    _hoverStatus = Common::String("picked mesh is not a geometry model");
                }
            }
        }
    }

    void GeometryInfoPanel::applyPickFilter(uint32_t mask)
    {
        _pickFilter = mask & GEOMETRY_FEATURE_MASK_ALL;
        IniManager::instance().setInt(GEOMETRY_PICK_FILTER_KEY, static_cast<int>(_pickFilter));

        EventPacket packet;
        packet.setData("mask", _pickFilter);
        CommandManager::instance().call(EventId::SetGeometryPickFilter, Common::move(packet));
    }

    void GeometryInfoPanel::renderPickFilter()
    {
        // Mirror the live mask so changes made through the command (for example by an AI
        // client) are reflected here.
        _pickFilter = RenderWork::instance().getGeometryPickFilter();

        ImGui::TextUnformatted("Pick filter (feature types)");
        const struct
        {
            uint32_t bit;
            const char* label;
            const char* hint;
        } options[] = {
            {GEOMETRY_FEATURE_MASK_VERTEX, "Vertex", "Vertex features"},
            {GEOMETRY_FEATURE_MASK_EDGE, "Edge", "Edge features"},
            {GEOMETRY_FEATURE_MASK_FACE, "Face",
             "Face features (the tessellated mesh resolves to faces)"},
            {GEOMETRY_FEATURE_MASK_SOLID, "Solid",
             "With Face disabled, a face hit is promoted to the solid that owns it"},
        };

        for (const auto& option : options)
        {
            bool enabled = (_pickFilter & option.bit) != 0;
            if (ImGui::Checkbox(option.label, &enabled))
            {
                applyPickFilter(enabled ? (_pickFilter | option.bit) : (_pickFilter & ~option.bit));
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", option.hint);
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();

        if (_pickFilter == 0)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f),
                               "No feature type is enabled: geometry picking is disabled.");
        }
        else
        {
            ImGui::Text("Active: %s", GeometryFeatureMaskInfo::toString(_pickFilter).c_str());
        }
    }

    void GeometryInfoPanel::configureWindow() const
    {
        ImGui::SetNextWindowSize(ImVec2(360, 320), ImGuiCond_FirstUseEver);
    }

    void GeometryInfoPanel::renderImpl()
    {
        if (_pickInfoSubscriptionId == 0)
        {
            _pickInfoSubscriptionId = RenderWork::instance().subscribePickInfo();
        }

        ImGui::Checkbox("Hover feature info", &_showHoverInfo);
        ImGui::TextWrapped("Hover a rendered face to inspect its B-Rep feature. Picking always "
                           "resolves to a complete feature, never to a single triangle.");
        ImGui::Separator();

        renderPickFilter();
        ImGui::Separator();

        updatePickedInfo();

        if (!_hasPickedInfo)
        {
            ImGui::TextWrapped("%s", _hoverStatus.size() == 0 ? "No feature picked."
                                                              : _hoverStatus.c_str());
        }
        else
        {
            ImGui::Text("Feature: %s", _featureInfo.name.c_str());
            ImGui::Text("Type: %s (%s)", featureTypeName(_featureInfo.ref.featureType).c_str(),
                        geometryTypeName(_featureInfo.geometryType).c_str());
            ImGui::Text("Feature id: %u   Elem id: %u", _featureInfo.featureId,
                        _featureInfo.elemId);
            ImGui::Separator();
            ImGui::Text("Length: %.6g", _featureInfo.length);
            ImGui::Text("Area:   %.6g", _featureInfo.area);
            ImGui::Text("Volume: %.6g", _featureInfo.volume);
            ImGui::Text("Center: (%.6g, %.6g, %.6g)", _featureInfo.x, _featureInfo.y,
                        _featureInfo.z);
            ImGui::Text("BBox min: (%.4g, %.4g, %.4g)", _featureInfo.bboxMin[0],
                        _featureInfo.bboxMin[1], _featureInfo.bboxMin[2]);
            ImGui::Text("BBox max: (%.4g, %.4g, %.4g)", _featureInfo.bboxMax[0],
                        _featureInfo.bboxMax[1], _featureInfo.bboxMax[2]);
            ImGui::Text("Parents: %u   Children: %u", _featureInfo.parentCount,
                        _featureInfo.childCount);
            ImGui::Text("Picked point: (%.6g, %.6g, %.6g)", _pickedInfo.x, _pickedInfo.y,
                        _pickedInfo.z);
        }

        syncPickRequest();
    }

    void GeometryInfoPanel::afterRenderFrame()
    {
        syncPickRequest();

        if (!_showHoverInfo || !_hasPickedInfo)
        {
            return;
        }

        // 鼠标位于任意 ImGui 窗口（本面板、其他面板、菜单）上时抑制 tooltip，
        // 仅在 3D 视口上悬停时展示。
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
        {
            return;
        }

        ImGui::BeginTooltip();
        ImGui::Text("%s", _featureInfo.name.c_str());
        ImGui::Text("%s", featureTypeName(_featureInfo.ref.featureType).c_str());
        ImGui::Text("Length: %.6g  Area: %.6g  Volume: %.6g", _featureInfo.length,
                    _featureInfo.area, _featureInfo.volume);
        ImGui::Text("Center: (%.6g, %.6g, %.6g)", _featureInfo.x, _featureInfo.y,
                    _featureInfo.z);
        ImGui::Text("Picked: (%.6g, %.6g, %.6g)", _pickedInfo.x, _pickedInfo.y, _pickedInfo.z);
        ImGui::EndTooltip();
    }

    void GeometryInfoPanel::abortClose()
    {
        releasePickRequest();
        if (_pickInfoSubscriptionId != 0)
        {
            RenderWork::instance().unsubscribePickInfo(_pickInfoSubscriptionId);
            _pickInfoSubscriptionId = 0;
        }
        _hasPickedInfo = false;
    }
} // namespace EgLab::Platform
