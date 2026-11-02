#include "MeshEngine/MeshGen/MidSurface/MidSurface.hpp"

#include "Common/Log.hpp"
#include "MeshEngine/MeshCheck/TypeChecker.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab::ME
{
    class MidSurface::Impl
    {
    public:
        Impl()
        {
        }
        ~Impl()
        {
        }

        Common::SharedPtr<Mesh> _input;
    };

    MidSurface::MidSurface(Common::SharedPtr<Mesh> _mesh) : _impl(Common::makeUnique<Impl>())
    {
        _impl->_input = _mesh;
    }

    MidSurface::~MidSurface()
    {
    }

    Common::Return MidSurface::generate()
    {
        if (_impl->_input == nullptr) return Common::Return::BadInput;
        TypeChecker checker(_impl->_input);
        if (checker.allTypeIs(ElemType::Tet4) == false)
        {
            LOG(ERROR) << "There are some non-tetrahedral elements!";
            return Common::Return::BadInput;
        }
        return Common::Return::Succeed;
    }

} // namespace EgLab::ME