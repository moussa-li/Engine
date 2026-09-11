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

        virtual Common::Return exec() override;
    };
} // namespace EgLab::Platform