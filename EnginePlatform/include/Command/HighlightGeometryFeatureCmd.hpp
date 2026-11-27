#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    /// Highlights one B-Rep feature of a GeometryData node; featureType 0xFFFFFFFF clears it.
    /// focus selects the "currently viewed feature" highlight colour instead of the selection
    /// highlight colour.
    class HighlightGeometryFeatureCmd : public CommandBase<GeometryFeatureHighlightParam>
    {
    public:
        explicit HighlightGeometryFeatureCmd(const GeometryFeatureHighlightParam& params,
                                            bool focus = false);

        Common::String getCmdName() const override
        {
            return Common::String(_focus ? "HighlightGeometryFeatureFocusCmd"
                                         : "HighlightGeometryFeatureCmd");
        }

        Common::Return execImpl() override;

    private:
        bool _focus{false};
    };
} // namespace EgLab::Platform
