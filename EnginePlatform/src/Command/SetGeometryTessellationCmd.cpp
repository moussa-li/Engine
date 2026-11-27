#include "Command/SetGeometryTessellationCmd.hpp"

#include "Common/Log.hpp"
#include "DataBase/Context.hpp"
#include "DataBase/DataTree.hpp"
#include "DataBase/GeometryData.hpp"
#include "DataBase/GeometryMeshBuild.hpp"
#include "DataBase/GeometryMeshConverter.hpp"
#include "GeometryEngine/Common/GeometryImportReport.hpp"
#include "GeometryEngine/IO/GeometryImporter.hpp"
#include "GeometryEngine/Kernel/GeometryKernelRegistry.hpp"
#include "GeometryEngine/Model/GeometryDocument.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"
#include "Work/GeometryTessellation.hpp"
#include "Work/RenderWork.hpp"

namespace EgLab::Platform
{
    SetGeometryTessellationCmd::SetGeometryTessellationCmd(const GeometryTessellationParam& params)
        : CommandBase<GeometryTessellationParam>(params)
    {
    }

    void SetGeometryTessellationCmd::collectGeometryData(
        const Common::DynamicArray<Common::SharedPtr<TreeNode>>& nodes,
        Common::DynamicArray<Common::SharedPtr<GeometryData>>& out) const
    {
        for (const auto& node : nodes)
        {
            auto geometryData = Common::dynamicSharedPtrCast<GeometryData>(node);
            if (geometryData)
            {
                out.pushBack(geometryData);
            }
            collectGeometryData(node->getChildren(), out);
        }
    }

    std::size_t SetGeometryTessellationCmd::retessellateExisting()
    {
        const GE::GeometryImportOptions options =
            GeometryTessellationSettings::instance().getOptions();

        Common::DynamicArray<Common::SharedPtr<GeometryData>> geometryNodes;
        collectGeometryData(Context::instance().getDataTree().getChildren(), geometryNodes);

        std::size_t rebuilt = 0;
        for (const auto& data : geometryNodes)
        {
            if (!data)
            {
                continue;
            }
            const Common::String sourcePath = data->getSourcePath();
            if (sourcePath.size() == 0)
            {
                LOG(WARNING) << "SetGeometryTessellationCmd: geometry node has no source path; "
                                "skipped";
                continue;
            }

            // Reuse the kernel that produced the model so the rebuilt mesh matches the import.
            Common::String kernelName = GE::GeometryKernelRegistry::getDefaultKernelName();
            if (auto document = data->getDocument())
            {
                const Common::String& previousKernel = document->getLastReport().getKernelName();
                if (previousKernel.size() > 0)
                {
                    kernelName = previousKernel;
                }
            }

            auto newDocument = Common::makeShared<GE::GeometryDocument>();
            GE::GeometryImporter importer(kernelName, options);
            const GE::GeometryImportReport report = importer.importFile(sourcePath, *newDocument);
            if (!report.isSucceed())
            {
                LOG(WARNING) << "SetGeometryTessellationCmd: re-import failed for "
                             << data->getName() << ": " << report.getMessage();
                continue;
            }

            GeometryMeshBuild build;
            if (GeometryMeshConverter::convert(newDocument, build) != Common::Return::Succeed ||
                !build.mesh)
            {
                LOG(WARNING) << "SetGeometryTessellationCmd: rebuilt mesh is empty for "
                             << data->getName();
                continue;
            }

            auto oldMesh = data->getMesh();
            data->setMesh(build.mesh);
            data->setDocument(newDocument);
            data->setElementFeatures(build.elementFeatures);
            data->setVertexEdgeFeatures(build.nodeFeatures, build.edgeSegmentIndices,
                                        build.edgeSegmentFeatures);
            data->setNodeUvs(build.nodeUvs);

            // Rebuild the render primitives for the new node/element topology: removal clears the
            // old selection/pick state and the update re-registers the new mesh.
            RenderWork::instance().removeMesh(oldMesh);
            RenderWork::instance().onUpdateMesh(build.mesh);
            ++rebuilt;
        }
        return rebuilt;
    }

    Common::Return SetGeometryTessellationCmd::execImpl()
    {
        const GeometryTessellationParam& params = getParams();
        GE::GeometryImportOptions options;
        options.setLinearDeflection(params.linearDeflection);
        options.setAngularDeflection(params.angularDeflection);
        options.setRelativeDeflection(params.relative != 0);
        options.setParallel(params.parallel != 0);

        GeometryTessellationSettings::instance().setOptions(options);
        const std::size_t rebuilt = retessellateExisting();

        LOG(INFO) << "SetGeometryTessellationCmd: linear=" << options.getLinearDeflection()
                  << ", angular=" << options.getAngularDeflection()
                  << ", relative=" << (options.isRelativeDeflection() ? 1 : 0)
                  << ", parallel=" << (options.isParallel() ? 1 : 0)
                  << ", rebuilt_models=" << rebuilt;
        return Common::Return::Succeed;
    }
} // namespace EgLab::Platform
