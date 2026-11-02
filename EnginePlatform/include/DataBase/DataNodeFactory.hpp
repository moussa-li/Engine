#pragma once

#include <atomic>
#include <type_traits>

#include "Common/SharedPtr.hpp"
#include "Common/Singleton.hpp"
#include "Common/String.hpp"
#include "DataBase/DataNode.hpp"

namespace EgLab::Platform
{
    class DataNodeFactory : public Common::Singleton<DataNodeFactory>
    {
    public:
        template <typename NodeType, typename... Args>
        Common::SharedPtr<NodeType> create(const Common::String& name, Args... args)
        {
            static_assert(std::is_base_of<DataNode, NodeType>::value,
                          "NodeType must derive from DataNode");
            const DataNodeId id = _nextId.fetch_add(1);
            return Common::makeShared<NodeType>(id, name, args...);
        }

    private:
        friend class Common::Singleton<DataNodeFactory>;

        DataNodeFactory() = default;

        std::atomic<DataNodeId> _nextId{1};
    };
} // namespace EgLab::Platform