#pragma once

#include <cstdint>

#include "Common/String.hpp"

namespace EgLab::Platform
{
    using DataNodeId = uint64_t;

    class DataNode
    {
    public:
        DataNode(DataNodeId id, const Common::String& name) : _id(id), _name(name)
        {
        }

        virtual ~DataNode() = default;

        DataNodeId getId() const
        {
            return _id;
        }

        const Common::String& getName() const
        {
            return _name;
        }

    private:
        DataNodeId _id;
        Common::String _name;
    };
} // namespace EgLab::Platform