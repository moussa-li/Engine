#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    // Locates a list of elements on one mesh: the requested elements stay highlighted while
    // every other face of that mesh is rendered transparent.
    class LocateElementsCmd : public CommandBase<LocateElementsParam>
    {
    public:
        explicit LocateElementsCmd(const LocateElementsParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("LocateElementsCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
