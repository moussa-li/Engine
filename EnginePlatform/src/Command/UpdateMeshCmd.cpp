#include "Command/UpdateMeshCmd.hpp"

#include <cstring>

#include "Common/Log.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    UpdateMeshCmd::UpdateMeshCmd(const UpdateMeshParam& params)
        : CommandBase<UpdateMeshParam>(params)
    {
    }

    UpdateMeshCmd::~UpdateMeshCmd()
    {
        if (getParams().data != nullptr)
        {
            delete[] getParams().data;
        }
    }

    Common::Return UpdateMeshCmd::execImpl()
    {
        if (getParams().dataSize == 0 || getParams().data == nullptr)
        {
            return Common::Return::BadInput;
        }

        // CommandBus thread-pool worker should not materialize OpenGL primitives.
        // It should only hand the payload to the dedicated render listener lane.
        RenderWork::instance().onUpdateMesh(getParams());

        LOG(INFO) << "UpdateMeshCmd::exec() handed off mesh payload to RenderWork";
        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform
