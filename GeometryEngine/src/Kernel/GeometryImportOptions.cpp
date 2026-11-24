#include "GeometryEngine/Kernel/GeometryImportOptions.hpp"

namespace EgLab::GE
{
    namespace
    {
        constexpr double MIN_LINEAR_DEFLECTION = 1.0e-9;

        constexpr double MIN_ANGULAR_DEFLECTION = 1.0e-6;
    } // namespace

    double GeometryImportOptions::getLinearDeflection() const
    {
        return _linearDeflection;
    }

    void GeometryImportOptions::setLinearDeflection(double deflection)
    {
        _linearDeflection = deflection > MIN_LINEAR_DEFLECTION ? deflection : MIN_LINEAR_DEFLECTION;
    }

    double GeometryImportOptions::getAngularDeflection() const
    {
        return _angularDeflection;
    }

    void GeometryImportOptions::setAngularDeflection(double deflection)
    {
        _angularDeflection = deflection > MIN_ANGULAR_DEFLECTION ? deflection : MIN_ANGULAR_DEFLECTION;
    }

    bool GeometryImportOptions::isRelativeDeflection() const
    {
        return _relativeDeflection;
    }

    void GeometryImportOptions::setRelativeDeflection(bool relative)
    {
        _relativeDeflection = relative;
    }

    bool GeometryImportOptions::isParallel() const
    {
        return _parallel;
    }

    void GeometryImportOptions::setParallel(bool parallel)
    {
        _parallel = parallel;
    }

} // namespace EgLab::GE
