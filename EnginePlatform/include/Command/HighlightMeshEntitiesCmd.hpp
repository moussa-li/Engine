#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    /// Highlights the nodes and/or elements of one MeshData node. Empty lists clear the
    /// highlight; both lists may be filled at once. focus selects the "current page" highlight
    /// colour instead of the row-selection colour.
    class HighlightMeshEntitiesCmd : public CommandBase<MeshEntityHighlightParam>
    {
    public:
        explicit HighlightMeshEntitiesCmd(const MeshEntityHighlightParam& params,
                                          bool focus = false);

        Common::String getCmdName() const override
        {
            return Common::String(_focus ? "HighlightMeshEntitiesFocusCmd"
                                         : "HighlightMeshEntitiesCmd");
        }

        Common::Return execImpl() override;

    private:
        bool _focus{false};
    };
} // namespace EgLab::Platform
