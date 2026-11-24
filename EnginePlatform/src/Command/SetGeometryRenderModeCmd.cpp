#include "Command/SetGeometryRenderModeCmd.hpp"

#include "Common/Log.hpp"
#include "Work/GeometryDisplayMode.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    SetGeometryRenderModeCmd::SetGeometryRenderModeCmd(const GeometryRenderModeParam& params)
        : CommandBase<GeometryRenderModeParam>(params)
    {
    }

    Common::Return SetGeometryRenderModeCmd::execImpl()
    {
        const uint32_t mode = getParams().mode;
        if (!GeometryDisplayModeInfo::isValid(mode))
        {
            LOG(WARNING) << "SetGeometryRenderModeCmd: unsupported mode " << mode;
            return Common::Return::BadInput;
        }

        RenderWork::instance().setGeometryDisplayMode(mode);
        LOG(INFO) << "SetGeometryRenderModeCmd: geometry display mode = "
                  << GeometryDisplayModeInfo::toString(static_cast<GeometryDisplayMode>(mode));
        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform
