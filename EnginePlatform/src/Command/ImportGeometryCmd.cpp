#include "Command/ImportGeometryCmd.hpp"

#include <cctype>
#include <filesystem>
#include <fstream>

#include "Common/Log.hpp"
#include "DataBase/Context.hpp"
#include "DataBase/DataNodeFactory.hpp"
#include "DataBase/GeometryData.hpp"
#include "DataBase/GeometryImportResultBuilder.hpp"
#include "DataBase/GeometryMeshConverter.hpp"
#include "DataBase/MeshDataQueryService.hpp"
#include "GeometryEngine/Common/GeometryImportReport.hpp"
#include "GeometryEngine/IO/GeometryImporter.hpp"
#include "GeometryEngine/Kernel/GeometryKernelRegistry.hpp"
#include "GeometryEngine/Model/GeometryDocument.hpp"
#include "GeometryEngine/Topology/Topology.hpp"
#include "GeometryEngine/Topology/TopologyType.hpp"
#include "Work/GeometryTessellation.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    namespace
    {
        bool isGeometryFile(const Common::String& extension)
        {
            return extension == ".stp" || extension == ".step" || extension == ".brp" ||
                   extension == ".brep";
        }
    } // namespace

    ImportGeometryCmd::ImportGeometryCmd(const ImportGeometryParam& params)
        : CommandBase<ImportGeometryParam>(params)
    {
    }

    Common::Return ImportGeometryCmd::execImpl()
    {
        auto& queryService = MeshDataQueryService::instance();

        const Common::String filePath(getParams().filePath);
        if (filePath.size() == 0)
        {
            queryService.publishGeometryImport(GeometryImportResultBuilder::makeFailure(1));
            return Common::Return::Succeed;
        }

        const std::filesystem::path path(filePath.c_str());
        Common::String extension(path.extension().string().c_str());
        for (std::size_t i = 0; i < extension.size(); ++i)
        {
            extension.begin()[i] = static_cast<char>(
                std::tolower(static_cast<unsigned char>(extension.begin()[i])));
        }
        if (!isGeometryFile(extension))
        {
            LOG(ERROR) << "ImportGeometryCmd: unsupported file extension: " << extension;
            queryService.publishGeometryImport(GeometryImportResultBuilder::makeFailure(1));
            return Common::Return::Succeed;
        }

        std::ifstream probe(filePath.c_str(), std::ios::binary);
        if (!probe.good())
        {
            LOG(ERROR) << "ImportGeometryCmd: file not found: " << filePath;
            queryService.publishGeometryImport(GeometryImportResultBuilder::makeFailure(1));
            return Common::Return::Succeed;
        }
        probe.close();

        // 导入 B-Rep 文档。
        Common::SharedPtr<GE::GeometryDocument> document =
            Common::makeShared<GE::GeometryDocument>();
        const Common::String kernelName(getParams().kernelName);
        GE::GeometryImporter importer(
            kernelName.size() == 0 ? GE::GeometryKernelRegistry::getDefaultKernelName()
                                   : kernelName,
            GeometryTessellationSettings::instance().getOptions());
        const GE::GeometryImportReport report = importer.importFile(filePath, *document);
        if (!report.isSucceed())
        {
            LOG(ERROR) << "ImportGeometryCmd: import failed: " << report.getMessage();
            queryService.publishGeometryImport(GeometryImportResultBuilder::makeFailure(2));
            return Common::Return::Succeed;
        }

        // 转换为渲染网格（Tri3）+ 面/边/顶点三层特征映射。
        GeometryMeshBuild build;
        if (GeometryMeshConverter::convert(document, build) != Common::Return::Succeed)
        {
            LOG(ERROR) << "ImportGeometryCmd: no renderable triangles in the model";
            queryService.publishGeometryImport(GeometryImportResultBuilder::makeFailure(3));
            return Common::Return::Succeed;
        }

        // 创建 GeometryData 数据节点。
        const Common::String nodeName(path.filename().string().c_str());
        auto& factory = DataNodeFactory::instance();
        auto geometryData =
            factory.create<GeometryData>(nodeName, build.mesh, document, filePath);
        geometryData->setElementFeatures(build.elementFeatures);
        geometryData->setVertexEdgeFeatures(build.nodeFeatures, build.edgeSegmentIndices,
                                            build.edgeSegmentFeatures);
        geometryData->setNodeUvs(build.nodeUvs);

        if (Context::instance().addDataNode(geometryData) != Common::Return::Succeed)
        {
            LOG(ERROR) << "ImportGeometryCmd: failed to register data node";
            queryService.publishGeometryImport(GeometryImportResultBuilder::makeFailure(4));
            return Common::Return::Succeed;
        }

        RenderWork::instance().onUpdateMesh(build.mesh);

        GeometryImportResult result = GeometryImportResultBuilder::makeSuccess(*geometryData, build);

        queryService.publishGeometryImport(result);
        LOG(INFO) << "ImportGeometryCmd: imported " << nodeName << " (faces=" << result.faceCount
                  << ", triangles=" << result.triangleCount << ", edges=" << result.edgeCount
                  << ", edge segments=" << build.edgeSegmentCount
                  << ", vertices=" << result.vertexCount << ")";
        return Common::Return::Succeed;
    }

} // namespace EgLab::Platform
