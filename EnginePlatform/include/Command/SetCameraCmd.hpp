#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    class SetCameraCmd : public CommandBase<SetCameraParam>
    {
    public:
        explicit SetCameraCmd(const SetCameraParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("SetCameraCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
