#include "Command/SetGeometryUvCmd.hpp"

#include "Common/Log.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    SetGeometryUvCmd::SetGeometryUvCmd(const GeometryUvParam& params)
        : CommandBase<GeometryUvParam>(params)
    {
    }

    Common::Return SetGeometryUvCmd::execImpl()
    {
        const bool enabled = getParams().enabled != 0;
        RenderWork::instance().setGeometryUv(enabled);
        LOG(INFO) << "SetGeometryUvCmd: surface UV display " << (enabled ? "enabled" : "disabled");
        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform
