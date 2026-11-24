#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    class ImportGeometryCmd : public CommandBase<ImportGeometryParam>
    {
    public:
        explicit ImportGeometryCmd(const ImportGeometryParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("ImportGeometryCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
