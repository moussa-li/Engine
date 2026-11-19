#pragma once

#include "Command/ICommand.hpp"

namespace EgLab::Platform
{
    // 网格数据精确查询命令：把 AI/外部调用者的查询请求转发给 MeshDataQueryService。
    // 每条命令对应一个 EventId，查询结果通过 "EgLab MeshDataResult-<pid>" 共享内存队列
    // 异步回传，由调用方（EgLab-Agent）读取。

    class ListDataTreeNodesCmd : public ICommand
    {
    public:
        Common::String getCmdName() const override
        {
            return Common::String("ListDataTreeNodesCmd");
        }

        Common::Return exec() override;
    };

    class GetMeshNodesCmd : public CommandBase<MeshQueryParam>
    {
    public:
        explicit GetMeshNodesCmd(const MeshQueryParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("GetMeshNodesCmd");
        }

        Common::Return execImpl() override;
    };

    class GetMeshElementsCmd : public CommandBase<MeshQueryParam>
    {
    public:
        explicit GetMeshElementsCmd(const MeshQueryParam& params);

        Common::String getCmdName() const override
        {
            return Common::String("GetMeshElementsCmd");
        }

        Common::Return execImpl() override;
    };

    class GetNodesXYZCmd : public CommandBase<MeshNodesQueryParam>
    {
    public:
        explicit GetNodesXYZCmd(const MeshNodesQueryParam& params);
        ~GetNodesXYZCmd() override;

        Common::String getCmdName() const override
        {
            return Common::String("GetNodesXYZCmd");
        }

        Common::Return execImpl() override;
    };

    class GetElementsTopologyCmd : public CommandBase<MeshElementsQueryParam>
    {
    public:
        explicit GetElementsTopologyCmd(const MeshElementsQueryParam& params);
        ~GetElementsTopologyCmd() override;

        Common::String getCmdName() const override
        {
            return Common::String("GetElementsTopologyCmd");
        }

        Common::Return execImpl() override;
    };
} // namespace EgLab::Platform
