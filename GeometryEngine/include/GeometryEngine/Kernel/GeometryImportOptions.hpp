#pragma once

#include "GeometryEngine/Common/GeometryAPI.hpp"

namespace EgLab::GE
{
    /// Discretization and parallelism settings forwarded to a kernel adapter.
    class GeometryEngineAPI GeometryImportOptions
    {
    public:
        GeometryImportOptions() = default;

        double getLinearDeflection() const;

        void setLinearDeflection(double deflection);

        double getAngularDeflection() const;

        void setAngularDeflection(double deflection);

        bool isRelativeDeflection() const;

        void setRelativeDeflection(bool relative);

        bool isParallel() const;

        void setParallel(bool parallel);

    private:
        double _linearDeflection{0.1};
        double _angularDeflection{0.5};
        bool _relativeDeflection{false};
        bool _parallel{true};
    };

} // namespace EgLab::GE
