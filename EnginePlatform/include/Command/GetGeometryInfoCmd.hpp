#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    class GetGeometryInfoCmd : public CommandBase<GeometryQueryParam>
    {
    public:
        explicit GetGeometryInfoCmd(const GeometryQueryParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("GetGeometryInfoCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
