#include "DataBase/DataTree.hpp"

#include <mutex>

namespace EgLab::Platform
{
    DataTree::DataTree(DataTree* parent) : DataTree("DataTree", parent)
    {
    }

    DataTree::DataTree(const Common::String& name, DataTree* parent)
        : _name(name), _parent(nullptr)
    {
        if (parent != nullptr)
        {
            parent->addTree(this);
        }
    }

    Common::Return DataTree::addTree(DataTree* child)
    {
        if (child == nullptr || child == this)
        {
            return Common::Return::BadInput;
        }

        std::scoped_lock lock(_childrenMutex, child->_childrenMutex);
        if (child->_parent != nullptr)
        {
            return Common::Return::Failed;
        }

        for (auto* ancestor = this; ancestor != nullptr; ancestor = ancestor->_parent)
        {
            if (ancestor == child)
            {
                return Common::Return::BadInput;
            }
        }

        _childrens.pushBack(child);
        child->_parent = this;
        return Common::Return::Succeed;
    }

    const Common::String& DataTree::getName() const
    {
        return _name;
    }

    Common::DynamicArray<DataTree*> DataTree::getChildrens() const
    {
        std::lock_guard<std::mutex> lock(_childrenMutex);
        return _childrens;
    }

    DataTree::~DataTree()
    {
        for (DataTree* child : _childrens)
        {
            delete child;
        }
    }
} // namespace EgLab::Platform
