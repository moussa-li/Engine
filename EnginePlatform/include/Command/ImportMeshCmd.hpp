#pragma once

#include "Command/ICommand.hpp"
#include "Common/String.hpp"

namespace EgLab::Platform
{

    class ImportMeshCmd : public CommandBase<ImportMeshParam>
    {
    public:
        ImportMeshCmd(const ImportMeshParam&);
        ~ImportMeshCmd();

        virtual Common::String getCmdName() const override
        {
            return Common::String("ImportMeshCmd");
        }

        virtual Common::Return execImpl() override;
    };
} // namespace EgLab::Platform