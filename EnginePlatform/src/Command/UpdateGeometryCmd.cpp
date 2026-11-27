#include "Command/UpdateGeometryCmd.hpp"

#include "Command/GeometryWire.hpp"
#include "Common/Log.hpp"
#include "DataBase/Context.hpp"
#include "DataBase/DataNodeFactory.hpp"
#include "DataBase/GeometryData.hpp"
#include "DataBase/GeometryImportResultBuilder.hpp"
#include "DataBase/GeometryMeshBuild.hpp"
#include "DataBase/GeometryMeshConverter.hpp"
#include "DataBase/MeshDataQueryService.hpp"
#include "GeometryEngine/Common/GeometryImportReport.hpp"
#include "GeometryEngine/IO/GeometryImporter.hpp"
#include "GeometryEngine/Kernel/GeometryKernelRegistry.hpp"
#include "GeometryEngine/Model/GeometryDocument.hpp"
#include "GeometryEngine/Topology/TopologyType.hpp"
#include "Work/GeometryTessellation.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    namespace
    {
        /// Reuses the kernel that produced an existing model so its rebuilt mesh stays
        /// comparable; a brand new node uses the configured default kernel.
        Common::String resolveKernelName(const Common::SharedPtr<GeometryData>& geometryData)
        {
            Common::String kernelName = GE::GeometryKernelRegistry::getDefaultKernelName();
            if (!geometryData)
            {
                return kernelName;
            }

            auto document = geometryData->getDocument();
            if (document)
            {
                const Common::String& previous = document->getLastReport().getKernelName();
                if (previous.size() > 0)
                {
                    kernelName = previous;
                }
            }
            return kernelName;
        }
    } // namespace

    UpdateGeometryCmd::UpdateGeometryCmd(const UpdateGeometryParam& params)
        : CommandBase<UpdateGeometryParam>(params)
    {
    }

    UpdateGeometryCmd::~UpdateGeometryCmd()
    {
        if (getParams().data != nullptr)
        {
            delete[] getParams().data;
        }
    }

    Common::Return UpdateGeometryCmd::execImpl()
    {
        auto& queryService = MeshDataQueryService::instance();
        const auto& params = getParams();

        if (params.data == nullptr || params.dataSize < sizeof(UpdateGeometryWireHeader))
        {
            LOG(ERROR) << "UpdateGeometryCmd: payload is too small for the wire header";
            queryService.publishGeometryImport(GeometryImportResultBuilder::makeFailure(1));
            return Common::Return::BadInput;
        }

        GeometryUpdateWire wire;
        if (!wire.parse(params.data, params.dataSize))
        {
            LOG(ERROR) << "UpdateGeometryCmd: invalid wire header or feature-name table";
            queryService.publishGeometryImport(GeometryImportResultBuilder::makeFailure(1));
            return Common::Return::BadInput;
        }

        const UpdateGeometryWireHeader& header = wire.getHeader();
        const Common::String nodeName(header.nodeName);
        auto& context = Context::instance();
        Common::SharedPtr<DataNode> existingNode;
        if (header.dataNodeId != 0)
        {
            existingNode = context.getDataNode(header.dataNodeId);
        }
        else if (nodeName.size() > 0)
        {
            existingNode = context.getDataNode(nodeName);
        }

        Common::SharedPtr<GeometryData> existingGeometry;
        if (existingNode)
        {
            existingGeometry = Common::dynamicSharedPtrCast<GeometryData>(existingNode);
            if (!existingGeometry)
            {
                LOG(ERROR) << "UpdateGeometryCmd: target node is not a GeometryData";
                queryService.publishGeometryImport(GeometryImportResultBuilder::makeFailure(1));
                return Common::Return::Failed;
            }
        }
        else if (header.dataNodeId != 0)
        {
            LOG(ERROR) << "UpdateGeometryCmd: data node " << header.dataNodeId
                       << " does not exist";
            queryService.publishGeometryImport(GeometryImportResultBuilder::makeFailure(1));
            return Common::Return::Failed;
        }

        if (!existingGeometry && nodeName.size() == 0)
        {
            LOG(ERROR) << "UpdateGeometryCmd: nodeName is required to create a geometry node";
            queryService.publishGeometryImport(GeometryImportResultBuilder::makeFailure(1));
            return Common::Return::BadInput;
        }

        Common::SharedPtr<GE::GeometryDocument> document =
            Common::makeShared<GE::GeometryDocument>();
        GE::GeometryImporter importer(resolveKernelName(existingGeometry),
                                      GeometryTessellationSettings::instance().getOptions());
        const GE::GeometryImportReport report =
            importer.importBuffer(wire.getTopoData(), wire.getTopoDataSize(), *document);
        if (!report.isSucceed())
        {
            LOG(ERROR) << "UpdateGeometryCmd: import failed: " << report.getMessage();
            queryService.publishGeometryImport(GeometryImportResultBuilder::makeFailure(2));
            return Common::Return::Failed;
        }

        for (std::size_t index = 0; index < wire.getNameCount(); ++index)
        {
            GeometryFeatureNameEntry entry{};
            wire.getNameEntry(index, entry);
            if (entry.featureType >= GE::TOPOLOGY_TYPE_COUNT)
            {
                LOG(WARNING) << "UpdateGeometryCmd: skipped a feature name with unknown type "
                             << entry.featureType;
                continue;
            }

            if (document->setFeatureName(static_cast<GE::TopologyType>(entry.featureType),
                                         entry.featureIndex,
                                         Common::String(entry.name)) !=
                Common::Return::Succeed)
            {
                LOG(WARNING) << "UpdateGeometryCmd: skipped feature name for missing feature "
                             << entry.featureType << ":" << entry.featureIndex;
            }
        }

        GeometryMeshBuild build;
        if (GeometryMeshConverter::convert(document, build) != Common::Return::Succeed ||
            !build.mesh)
        {
            LOG(ERROR) << "UpdateGeometryCmd: no renderable triangles in the model";
            queryService.publishGeometryImport(GeometryImportResultBuilder::makeFailure(3));
            return Common::Return::Failed;
        }

        Common::SharedPtr<GeometryData> geometryData = existingGeometry;
        if (geometryData)
        {
            auto previousMesh = geometryData->getMesh();
            geometryData->setMesh(build.mesh);
            geometryData->setDocument(document);
            // The new shape only exists in memory, so drop the stale file source: a later
            // retessellation must skip this model instead of reverting to the old file.
            geometryData->setSourcePath(Common::String());
            geometryData->setElementFeatures(build.elementFeatures);
            geometryData->setVertexEdgeFeatures(build.nodeFeatures, build.edgeSegmentIndices,
                                                build.edgeSegmentFeatures);
            geometryData->setNodeUvs(build.nodeUvs);

            // Removal clears the old selection/pick state before the new topology registers.
            if (previousMesh)
            {
                RenderWork::instance().removeMesh(previousMesh);
            }
            RenderWork::instance().onUpdateMesh(build.mesh);
        }
        else
        {
            // The shape arrives serialized in memory, so the node has no re-readable source path.
            geometryData = DataNodeFactory::instance().create<GeometryData>(
                nodeName, build.mesh, document, Common::String());
            geometryData->setElementFeatures(build.elementFeatures);
            geometryData->setVertexEdgeFeatures(build.nodeFeatures, build.edgeSegmentIndices,
                                                build.edgeSegmentFeatures);
            geometryData->setNodeUvs(build.nodeUvs);

            if (context.addDataNode(geometryData) != Common::Return::Succeed)
            {
                LOG(ERROR) << "UpdateGeometryCmd: failed to register data node";
                queryService.publishGeometryImport(GeometryImportResultBuilder::makeFailure(4));
                return Common::Return::Failed;
            }
            RenderWork::instance().onUpdateMesh(build.mesh);
        }

        const GeometryImportResult result =
            GeometryImportResultBuilder::makeSuccess(*geometryData, build);
        queryService.publishGeometryImport(result);

        LOG(INFO) << "UpdateGeometryCmd: " << (existingGeometry ? "updated " : "created ")
                  << geometryData->getName() << " (faces=" << result.faceCount
                  << ", triangles=" << result.triangleCount << ", edges=" << result.edgeCount
                  << ", vertices=" << result.vertexCount
                  << ", named features=" << wire.getNameCount() << ")";
        return Common::Return::Succeed;
    }

} // namespace EgLab::Platform
