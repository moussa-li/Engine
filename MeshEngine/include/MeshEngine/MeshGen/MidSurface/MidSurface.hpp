#pragma once

#include "Common/Return.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/UniquePtr.hpp"

namespace EgLab::ME
{
    class Mesh;

    class MidSurface
    {
    public:
        MidSurface(Common::SharedPtr<Mesh>);

        ~MidSurface();

        Common::Return generate();

    private:
        class Impl;
        Common::UniquePtr<Impl> _impl;
    };
} // namespace EgLab::ME