#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    /// Applies the feature type filter used by geometry picking and rectangle selection.
    class SetGeometryPickFilterCmd : public CommandBase<GeometryPickFilterParam>
    {
    public:
        explicit SetGeometryPickFilterCmd(const GeometryPickFilterParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("SetGeometryPickFilterCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
