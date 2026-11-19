#include "Command/MeshQueryCmd.hpp"

#include "DataBase/MeshDataQueryService.hpp"

namespace EgLab::Platform
{
    ListDataTreeNodesCmd::ListDataTreeNodesCmd(const ListDataTreeParam& params)
        : CommandBase<ListDataTreeParam>(params)
    {
    }

    Common::Return ListDataTreeNodesCmd::execImpl()
    {
        return MeshDataQueryService::instance().queryListDataTree(getParams());
    }

    GetMeshNodesCmd::GetMeshNodesCmd(const MeshQueryParam& params)
        : CommandBase<MeshQueryParam>(params)
    {
    }

    Common::Return GetMeshNodesCmd::execImpl()
    {
        return MeshDataQueryService::instance().queryMeshNodes(getParams());
    }

    GetMeshElementsCmd::GetMeshElementsCmd(const MeshQueryParam& params)
        : CommandBase<MeshQueryParam>(params)
    {
    }

    Common::Return GetMeshElementsCmd::execImpl()
    {
        return MeshDataQueryService::instance().queryMeshElements(getParams());
    }

    GetNodesXYZCmd::GetNodesXYZCmd(const MeshNodesQueryParam& params)
        : CommandBase<MeshNodesQueryParam>(params)
    {
    }

    Common::Return GetNodesXYZCmd::execImpl()
    {
        return MeshDataQueryService::instance().queryNodesXYZ(getParams());
    }

    GetElementsTopologyCmd::GetElementsTopologyCmd(const MeshElementsQueryParam& params)
        : CommandBase<MeshElementsQueryParam>(params)
    {
    }

    Common::Return GetElementsTopologyCmd::execImpl()
    {
        return MeshDataQueryService::instance().queryElementsTopology(getParams());
    }
} // namespace EgLab::Platform
