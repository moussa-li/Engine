#pragma once

#include "Common/HashSet.hpp"
#include "Common/Return.hpp"

namespace EgLab::Platform
{
    class DataTree
    {
    public:
        DataTree(DataTree* parent);

        Common::Return addTree(DataTree*);

        const Common::DynamicArray<DataTree*>& getChildrens() const;
        virtual ~DataTree();

    private:
        DataTree* _parent;
        Common::HashSet<DataTree*> _childrens;
    };
} // namespace EgLab::Platform