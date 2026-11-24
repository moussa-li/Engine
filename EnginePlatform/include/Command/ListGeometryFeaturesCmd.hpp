#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    class ListGeometryFeaturesCmd : public CommandBase<GeometryFeaturesParam>
    {
    public:
        explicit ListGeometryFeaturesCmd(const GeometryFeaturesParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("ListGeometryFeaturesCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
