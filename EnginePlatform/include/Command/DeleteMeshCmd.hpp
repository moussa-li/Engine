#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    class DeleteMeshCmd : public CommandBase<DeleteMeshParam>
    {
    public:
        explicit DeleteMeshCmd(const DeleteMeshParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("DeleteMeshCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform