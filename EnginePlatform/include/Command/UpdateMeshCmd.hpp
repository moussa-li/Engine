#pragma once

#include "Command/ICommand.hpp"
#include "Command/CommandParam.hpp"

namespace EgLab::Platform
{
    class UpdateMeshCmd : public CommandBase<UpdateMeshParam>
    {
    public:
        explicit UpdateMeshCmd(const UpdateMeshParam& params);
        ~UpdateMeshCmd();

        Common::Return exec() override;
    };
} // namespace EgLab::Platform
