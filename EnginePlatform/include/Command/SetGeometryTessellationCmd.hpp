#pragma once

#include <cstddef>

#include "Command/ICommand.hpp"
#include "Common/SharedPtr.hpp"

namespace EgLab::Platform
{
    class GeometryData;
    class TreeNode;

    /// Updates the B-Rep tessellation parameters and re-tessellates every imported geometry
    /// model with them, so the render mesh density changes immediately.
    class SetGeometryTessellationCmd : public CommandBase<GeometryTessellationParam>
    {
    public:
        explicit SetGeometryTessellationCmd(const GeometryTessellationParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("SetGeometryTessellationCmd");
        }

        Common::Return execImpl() override;

    private:
        /// Re-imports every geometry data node with the current options and swaps its render
        /// mesh.
        /// @return Number of models that were rebuilt.
        std::size_t retessellateExisting();

        void collectGeometryData(
            const Common::DynamicArray<Common::SharedPtr<TreeNode>>& nodes,
            Common::DynamicArray<Common::SharedPtr<GeometryData>>& out) const;
    };
} // namespace EgLab::Platform
