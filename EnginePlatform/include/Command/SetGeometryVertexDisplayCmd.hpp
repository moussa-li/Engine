#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    /// Shows or hides the B-Rep vertex markers of imported geometry models.
    class SetGeometryVertexDisplayCmd : public CommandBase<GeometryVertexDisplayParam>
    {
    public:
        explicit SetGeometryVertexDisplayCmd(const GeometryVertexDisplayParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("SetGeometryVertexDisplayCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
