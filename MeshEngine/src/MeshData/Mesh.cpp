#include "MeshEngine/MeshData/Mesh.hpp"

#include "Common/HashMap.hpp"

namespace EgLab::ME
{
    enum class NodeStatus : int
    {
        Delete = -1,
        Normal = 0
    };

    enum class ElemStatus : int
    {
        Delete = -1,
        Normal = 0
    };

    class Mesh::Impl
    {
    public:
        Common::DynamicArray<Node> _nodes;
        Common::DynamicArray<NodeStatus> _nodeStatus;
        Common::HashMap<IdType, IdxType> _nodeIdToIdx;

        Common::DynamicArray<Elem> _elems;
        Common::DynamicArray<ElemStatus> _elemStatus;
        Common::HashMap<IdType, IdxType> _elemIdToIdx;
    };

    Mesh::Mesh() : _impl(Common::makeUnique<Impl>())
    {
    }

    Mesh::~Mesh()
    {
    }

    Common::Return Mesh::addNode(Node &&n)
    {
        if (_impl->_nodeIdToIdx.find(n.getId()) != _impl->_nodeIdToIdx.end())
        {
            LOG(ERROR) << "mesh::node already added!";
            return Common::Return::BadInput;
        }
        _impl->_nodeIdToIdx[n.getId()] = _impl->_nodes.size();
        _impl->_nodes.pushBack(Common::move(n));
        _impl->_nodeStatus.pushBack(NodeStatus::Normal);

        return Common::Return::Succeed;
    }

    Common::Return Mesh::delNode(IdType id)
    {
        if (_impl->_nodeIdToIdx.find(id) == _impl->_nodeIdToIdx.end())
        {
            LOG(ERROR) << "mesh::node not added!";
            return Common::Return::BadInput;
        }

        _impl->_nodeStatus[_impl->_nodeIdToIdx[id]] = NodeStatus::Delete;

        return Common::Return::Succeed;
    }

    Common::Return Mesh::addElem(Elem &&e)
    {
        if (_impl->_elemIdToIdx.find(e.getId()) != _impl->_elemIdToIdx.end())
        {
            LOG(ERROR) << "mesh::elem already added!";
            return Common::Return::BadInput;
        }
        _impl->_elemIdToIdx[e.getId()] = _impl->_nodes.size();
        _impl->_elems.pushBack(Common::move(e));
        _impl->_elemStatus.pushBack(ElemStatus::Normal);

        return Common::Return::Succeed;
    }

    Common::Return Mesh::delElem(IdType id)
    {
        if (_impl->_elemIdToIdx.find(id) == _impl->_elemIdToIdx.end())
        {
            LOG(ERROR) << "mesh::elem not added!";
            return Common::Return::BadInput;
        }

        _impl->_elemStatus[_impl->_elemIdToIdx[id]] = ElemStatus::Delete;

        return Common::Return::Succeed;
    }

    class MeshIterator::Impl
    {
    public:
        Impl(Common::UniquePtr<Mesh::Impl> &mesh)
            : _mesh(mesh),
              _nodeIt(mesh->_nodes.begin()),
              _elemIt(mesh->_elems.begin()),
              _nodeStatusIt(mesh->_nodeStatus.begin()),
              _elemStatusIt(mesh->_elemStatus.begin())
        {
        }
        Common::UniquePtr<Mesh::Impl> &_mesh;

        Common::DynamicArray<Node>::DynamicArrayIterator _nodeIt;
        Common::DynamicArray<NodeStatus>::DynamicArrayIterator _nodeStatusIt;
        Common::DynamicArray<Elem>::DynamicArrayIterator _elemIt;
        Common::DynamicArray<ElemStatus>::DynamicArrayIterator _elemStatusIt;
    };

    MeshIterator::MeshIterator(Mesh &mesh) : _impl(Common::makeUnique<Impl>(mesh._impl))
    {
    }

    MeshIterator::~MeshIterator()
    {
    }

    const Node &MeshIterator::currentNode() const
    {
        return *_impl->_nodeIt;
    }

    Node &MeshIterator::currentNode()
    {
        return *_impl->_nodeIt;
    }

    bool MeshIterator::nextNode()
    {
        do
        {
            _impl->_nodeStatusIt.next();
            _impl->_nodeIt.next();
        } while (_impl->_nodeIt.hasNext() && *_impl->_nodeStatusIt == NodeStatus::Delete);

        return _impl->_nodeIt != _impl->_mesh->_nodes.end();
    }

    const Elem &MeshIterator::currentElem() const
    {
        return *_impl->_elemIt;
    }

    Elem &MeshIterator::currentElem()
    {
        return *_impl->_elemIt;
    }

    bool MeshIterator::nextElem()
    {
        do
        {
            _impl->_elemStatusIt.next();
            _impl->_elemIt.next();
        } while (_impl->_elemIt.hasNext() && *_impl->_elemStatusIt == ElemStatus::Delete);
        return _impl->_elemIt != _impl->_mesh->_elems.end();
    }

} // namespace EgLab::ME
