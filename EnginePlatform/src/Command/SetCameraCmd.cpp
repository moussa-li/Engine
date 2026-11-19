#include "Command/SetCameraCmd.hpp"

#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    SetCameraCmd::SetCameraCmd(const SetCameraParam& params)
        : CommandBase<SetCameraParam>(params)
    {
    }

    Common::Return SetCameraCmd::execImpl()
    {
        RenderWork::instance().setCamera(getParams());
        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform
