#include "Command/ListGeometryFeaturesCmd.hpp"

#include "DataBase/MeshDataQueryService.hpp"

namespace EgLab::Platform
{
    ListGeometryFeaturesCmd::ListGeometryFeaturesCmd(const GeometryFeaturesParam& params)
        : CommandBase<GeometryFeaturesParam>(params)
    {
    }

    Common::Return ListGeometryFeaturesCmd::execImpl()
    {
        return MeshDataQueryService::instance().queryGeometryFeatures(getParams());
    }
} // namespace EgLab::Platform
