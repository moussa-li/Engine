#include "GeometryEngine/Topology/TopologyNode.hpp"

namespace EgLab::GE
{
    TopologyNode::TopologyNode() = default;

    TopologyNode::TopologyNode(IdType id, const Common::String& name, TopologyType type)
        : _id(id), _name(name), _type(type)
    {
    }

    IdType TopologyNode::getId() const
    {
        return _id;
    }

    void TopologyNode::setId(IdType id)
    {
        _id = id;
    }

    const Common::String& TopologyNode::getName() const
    {
        return _name;
    }

    void TopologyNode::setName(const Common::String& name)
    {
        _name = name;
    }

    TopologyType TopologyNode::getType() const
    {
        return _type;
    }

    const Common::DynamicArray<IdxType>& TopologyNode::getChildren() const
    {
        return _children;
    }

    const Common::DynamicArray<IdxType>& TopologyNode::getParents() const
    {
        return _parents;
    }

    bool TopologyNode::hasChild(IdxType childIndex) const
    {
        for (std::size_t i = 0; i < _children.size(); ++i)
        {
            if (_children[i] == childIndex) return true;
        }
        return false;
    }

    bool TopologyNode::hasParent(IdxType parentIndex) const
    {
        for (std::size_t i = 0; i < _parents.size(); ++i)
        {
            if (_parents[i] == parentIndex) return true;
        }
        return false;
    }

    void TopologyNode::addChild(IdxType childIndex)
    {
        if (childIndex == INVALID_IDX || hasChild(childIndex)) return;
        _children.pushBack(childIndex);
    }

    void TopologyNode::addParent(IdxType parentIndex)
    {
        if (parentIndex == INVALID_IDX || hasParent(parentIndex)) return;
        _parents.pushBack(parentIndex);
    }

    void TopologyNode::clearRelations()
    {
        _children.clear();
        _parents.clear();
    }

} // namespace EgLab::GE
