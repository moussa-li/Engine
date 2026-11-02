#pragma once

#include "Common/SharedPtr.hpp"
#include "Common/UniquePtr.hpp"
#include "MeshEngine/MeshData/Element.hpp"

namespace EgLab::ME
{
    class Mesh;
    class TypeChecker
    {
    public:
        TypeChecker(Common::SharedPtr<Mesh>);

        ~TypeChecker();

        bool allTypeIs(ElemType) const;

    private:
        class Impl;
        Common::UniquePtr<Impl> _impl;
    };
} // namespace EgLab::ME