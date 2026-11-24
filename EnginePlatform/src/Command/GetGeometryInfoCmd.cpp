#include "Command/GetGeometryInfoCmd.hpp"

#include "DataBase/MeshDataQueryService.hpp"

namespace EgLab::Platform
{
    GetGeometryInfoCmd::GetGeometryInfoCmd(const GeometryQueryParam& params)
        : CommandBase<GeometryQueryParam>(params)
    {
    }

    Common::Return GetGeometryInfoCmd::execImpl()
    {
        return MeshDataQueryService::instance().queryGeometryInfo(getParams());
    }
} // namespace EgLab::Platform
