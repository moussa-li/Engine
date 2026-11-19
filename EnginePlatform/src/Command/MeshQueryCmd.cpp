#include "Command/MeshQueryCmd.hpp"

#include "DataBase/MeshDataQueryService.hpp"

namespace EgLab::Platform
{
    Common::Return ListDataTreeNodesCmd::exec()
    {
        return MeshDataQueryService::instance().queryListDataTree();
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

    GetNodesXYZCmd::~GetNodesXYZCmd()
    {
        if (getParams().nodeIds != nullptr)
        {
            delete[] getParams().nodeIds;
        }
    }

    Common::Return GetNodesXYZCmd::execImpl()
    {
        return MeshDataQueryService::instance().queryNodesXYZ(getParams());
    }

    GetElementsTopologyCmd::GetElementsTopologyCmd(const MeshElementsQueryParam& params)
        : CommandBase<MeshElementsQueryParam>(params)
    {
    }

    GetElementsTopologyCmd::~GetElementsTopologyCmd()
    {
        if (getParams().elemIds != nullptr)
        {
            delete[] getParams().elemIds;
        }
    }

    Common::Return GetElementsTopologyCmd::execImpl()
    {
        return MeshDataQueryService::instance().queryElementsTopology(getParams());
    }
} // namespace EgLab::Platform
