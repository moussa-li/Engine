#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    /// Turns the zebra (isophote) stripe analysis of imported geometry faces on or off.
    class SetGeometryZebraCmd : public CommandBase<GeometryZebraParam>
    {
    public:
        explicit SetGeometryZebraCmd(const GeometryZebraParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("SetGeometryZebraCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
