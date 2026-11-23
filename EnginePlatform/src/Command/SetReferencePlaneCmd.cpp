#include "Command/SetReferencePlaneCmd.hpp"

#include "Core/IniManager.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    SetReferencePlaneCmd::SetReferencePlaneCmd(const SetReferencePlaneParam& params)
        : CommandBase<SetReferencePlaneParam>(params)
    {
    }

    Common::Return SetReferencePlaneCmd::execImpl()
    {
        const SetReferencePlaneParam& param = getParams();
        if (param.fieldMask == 0)
        {
            return Common::Return::Succeed;
        }

        RenderWork::instance().setReferencePlaneSettings(param);

        auto& ini = IniManager::instance();
        if ((param.fieldMask & REFERENCE_PLANE_FIELD_ENABLED) != 0)
        {
            ini.setBool("referencePlane.enabled", param.enabled != 0);
        }
        if ((param.fieldMask & REFERENCE_PLANE_FIELD_SHOW_GRID) != 0)
        {
            ini.setBool("referencePlane.showGrid", param.showGrid != 0);
        }
        if ((param.fieldMask & REFERENCE_PLANE_FIELD_PLACEMENT) != 0)
        {
            ini.setInt("referencePlane.placement", static_cast<int>(param.placement));
        }
        if ((param.fieldMask & REFERENCE_PLANE_FIELD_SCALE) != 0)
        {
            ini.setFloat("referencePlane.scale", param.scale);
        }

        static const char* colorAxes[3] = {"x", "y", "z"};
        const uint32_t colorMasks[3] = {REFERENCE_PLANE_FIELD_COLOR_X,
                                        REFERENCE_PLANE_FIELD_COLOR_Y,
                                        REFERENCE_PLANE_FIELD_COLOR_Z};
        for (int plane = 0; plane < 3; ++plane)
        {
            if ((param.fieldMask & colorMasks[plane]) == 0)
            {
                continue;
            }
            const Common::String prefix =
                Common::String("referencePlane.") + colorAxes[plane];
            ini.setFloat(prefix + ".r", param.colors[plane][0]);
            ini.setFloat(prefix + ".g", param.colors[plane][1]);
            ini.setFloat(prefix + ".b", param.colors[plane][2]);
            ini.setFloat(prefix + ".a", param.colors[plane][3]);
        }

        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform
