#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    /// Applies a partial update to the reference plane appearance.
    class SetReferencePlaneCmd : public CommandBase<SetReferencePlaneParam>
    {
    public:
        explicit SetReferencePlaneCmd(const SetReferencePlaneParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("SetReferencePlaneCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
