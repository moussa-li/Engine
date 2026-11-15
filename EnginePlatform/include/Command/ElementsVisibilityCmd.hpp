#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    // Shows or hides the requested elements of one mesh. For volume meshes the boundary
    // surface is recomputed so that interior faces whose neighbour became hidden are
    // revealed as new boundary faces.
    class ElementsVisibilityCmd : public CommandBase<ElementsVisibilityParam>
    {
    public:
        explicit ElementsVisibilityCmd(const ElementsVisibilityParam& params);
        ~ElementsVisibilityCmd() override;

        Common::String getCmdName() const override
        {
            return Common::String("ElementsVisibilityCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
