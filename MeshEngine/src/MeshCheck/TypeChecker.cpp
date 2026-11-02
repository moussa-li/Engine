#include "MeshEngine/MeshCheck/TypeChecker.hpp"

#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab::ME
{

    class TypeChecker::Impl
    {
    public:
        Impl()
        {
        }

        ~Impl()
        {
        }

        Common::SharedPtr<Mesh> _mesh;
    };

    TypeChecker::TypeChecker(Common::SharedPtr<Mesh> mesh) : _impl(Common::makeUnique<Impl>())
    {
        _impl->_mesh = mesh;
    }

    TypeChecker::~TypeChecker()
    {
    }

    bool TypeChecker::allTypeIs(ElemType type) const
    {
        if (_impl->_mesh == nullptr) return false;
        MeshIterator it(*_impl->_mesh);

        for (; it.hasMoreElem(); it.nextElem())
        {
            auto& elem = it.currentElem();
            if (elem.getType() != type) return false;
        }
        return true;
    }

} // namespace EgLab::ME