#include "Command/SetGeometryPickFilterCmd.hpp"

#include "Common/Log.hpp"
#include "DataBase/GeometryFeatureMask.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    SetGeometryPickFilterCmd::SetGeometryPickFilterCmd(const GeometryPickFilterParam& params)
        : CommandBase<GeometryPickFilterParam>(params)
    {
    }

    Common::Return SetGeometryPickFilterCmd::execImpl()
    {
        const uint32_t mask = getParams().mask & GEOMETRY_FEATURE_MASK_ALL;
        RenderWork::instance().setGeometryPickFilter(mask);
        LOG(INFO) << "SetGeometryPickFilterCmd: feature mask = "
                  << GeometryFeatureMaskInfo::toString(mask).c_str();
        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform
