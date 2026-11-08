#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    // Replaces the highlighted mesh set from a list of data node ids.
    class HighlightCmd : public CommandBase<HighlightParam>
    {
    public:
        explicit HighlightCmd(const HighlightParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("HighlightCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
