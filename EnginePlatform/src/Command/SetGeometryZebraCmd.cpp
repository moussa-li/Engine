#include "Command/SetGeometryZebraCmd.hpp"

#include "Common/Log.hpp"
#include "Work/GeometryZebra.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    SetGeometryZebraCmd::SetGeometryZebraCmd(const GeometryZebraParam& params)
        : CommandBase<GeometryZebraParam>(params)
    {
    }

    Common::Return SetGeometryZebraCmd::execImpl()
    {
        const GeometryZebraParam& params = getParams();
        const bool enabled = params.enabled != 0;
        const float frequency = GeometryZebra::clampFrequency(params.frequency);
        const Common::Vector3f lightDirection = GeometryZebra::clampDirection(
            Common::Vector3f(params.lightDirection[0], params.lightDirection[1],
                             params.lightDirection[2]));

        RenderWork::instance().setGeometryZebra(enabled, frequency, lightDirection);
        LOG(INFO) << "SetGeometryZebraCmd: geometry zebra stripes "
                  << (enabled ? "enabled" : "disabled") << ", frequency=" << frequency
                  << ", light=(" << lightDirection.x() << ", " << lightDirection.y() << ", "
                  << lightDirection.z() << ")";
        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform
