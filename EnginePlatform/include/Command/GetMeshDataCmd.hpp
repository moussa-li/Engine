#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    class GetMeshDataCmd : public CommandBase<GetMeshDataParam>
    {
    public:
        explicit GetMeshDataCmd(const GetMeshDataParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("GetMeshDataCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
