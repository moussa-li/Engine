#pragma once

#include "Command/CommandParam.hpp"
#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    class UpdateMeshCmd : public CommandBase<UpdateMeshParam>
    {
    public:
        explicit UpdateMeshCmd(const UpdateMeshParam& params);
        ~UpdateMeshCmd();

        virtual Common::String getCmdName() const override
        {
            return Common::String("UpdateMeshCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
