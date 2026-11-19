#include "Command/GetMeshDataCmd.hpp"

#include "DataBase/MeshDataQueryService.hpp"

namespace EgLab::Platform
{
    GetMeshDataCmd::GetMeshDataCmd(const GetMeshDataParam& params)
        : CommandBase<GetMeshDataParam>(params)
    {
    }

    Common::Return GetMeshDataCmd::execImpl()
    {
        return MeshDataQueryService::instance().queryAndPublish(getParams().dataNodeId);
    }
} // namespace EgLab::Platform
