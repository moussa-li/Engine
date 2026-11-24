#include "Command/SetGeometryVertexDisplayCmd.hpp"

#include "Common/Log.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    SetGeometryVertexDisplayCmd::SetGeometryVertexDisplayCmd(
        const GeometryVertexDisplayParam& params)
        : CommandBase<GeometryVertexDisplayParam>(params)
    {
    }

    Common::Return SetGeometryVertexDisplayCmd::execImpl()
    {
        const bool visible = getParams().visible != 0;
        RenderWork::instance().setGeometryVertexDisplay(visible);
        LOG(INFO) << "SetGeometryVertexDisplayCmd: geometry vertices "
                  << (visible ? "shown" : "hidden");
        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform
