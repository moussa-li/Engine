#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    // Shows or hides the requested nodes of one mesh. Node indices used by the render
    // primitives never change: hidden nodes are degenerated in the node shader so picking and
    // selection highlight indices stay valid.
    class NodeVisibilityCmd : public CommandBase<NodeVisibilityParam>
    {
    public:
        explicit NodeVisibilityCmd(const NodeVisibilityParam& params);
        ~NodeVisibilityCmd() override;

        Common::String getCmdName() const override
        {
            return Common::String("NodeVisibilityCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
