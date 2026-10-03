#pragma once

#include <mutex>

#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "Common/String.hpp"

namespace EgLab::Platform
{
    class DataTree
    {
    public:
        DataTree(DataTree* parent);
        explicit DataTree(const Common::String& name, DataTree* parent = nullptr);
        DataTree(const DataTree&) = delete;
        DataTree& operator=(const DataTree&) = delete;

        Common::Return addTree(DataTree*);

        const Common::String& getName() const;
        Common::DynamicArray<DataTree*> getChildrens() const;
        virtual ~DataTree();

    private:
        Common::String _name;
        DataTree* _parent;
        mutable std::mutex _childrenMutex;
        Common::DynamicArray<DataTree*> _childrens;
    };
} // namespace EgLab::Platform
