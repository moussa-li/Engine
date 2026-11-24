#include "Panel/GeometryInfoPanel.hpp"

#include <sstream>

#include "Work/RenderWork.hpp"
#include "imgui.h"

namespace EgLab::Platform
{
    GeometryInfoPanel::GeometryInfoPanel() = default;

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
            if (pickedInfo.kind == PickedMeshKind::ElementFace)
            {
                auto geometryData = GeometryData::findByMesh(pickedInfo.meshId);
                if (geometryData)
                {
                    _featureInfo = geometryData->getFeatureInfo(pickedInfo.entityId);
                    if (_featureInfo.ref.isValid())
                    {
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
        ImGui::TextWrapped("Hover a rendered face to inspect its B-Rep feature.");
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
