#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    /// Turns the surface-UV colouring of imported geometry faces on or off.
    class SetGeometryUvCmd : public CommandBase<GeometryUvParam>
    {
    public:
        explicit SetGeometryUvCmd(const GeometryUvParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("SetGeometryUvCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
