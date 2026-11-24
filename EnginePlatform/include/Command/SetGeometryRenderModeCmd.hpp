#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    class SetGeometryRenderModeCmd : public CommandBase<GeometryRenderModeParam>
    {
    public:
        explicit SetGeometryRenderModeCmd(const GeometryRenderModeParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("SetGeometryRenderModeCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
