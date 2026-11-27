#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    /// Highlights one B-Rep feature of a GeometryData node; featureType 0xFFFFFFFF clears it.
    class HighlightGeometryFeatureCmd : public CommandBase<GeometryFeatureHighlightParam>
    {
    public:
        explicit HighlightGeometryFeatureCmd(const GeometryFeatureHighlightParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("HighlightGeometryFeatureCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
