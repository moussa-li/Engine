#pragma once

#include <cstdint>
#include <mutex>

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/String.hpp"
#include "DataBase/MeshData.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab::GE
{
    class GeometryDocument;
}

namespace EgLab::Platform
{
    /// Identifies one feature inside a GeometryData topology.
    struct GeometryFeatureRef
    {
        uint32_t featureType{0xFFFFFFFFu};  // GE::TopologyType; 0xFFFFFFFF = invalid
        uint32_t featureIndex{0xFFFFFFFFu}; // GE::IdxType; 0xFFFFFFFF = invalid

        bool isValid() const
        {
            return featureType != 0xFFFFFFFFu && featureIndex != 0xFFFFFFFFu;
        }
    };

    /// Flattened, index-only description of one topological feature.
    struct GeometryFeatureInfo
    {
        GeometryFeatureRef ref;
        uint32_t featureId{0xFFFFFFFFu};
        uint32_t geometryType{0};      // GE::GeometryType
        uint32_t parentCount{0};
        uint32_t childCount{0};
        uint32_t elemId{0xFFFFFFFFu};
        double length{0.0};
        double area{0.0};
        double volume{0.0};
        double x{0.0};                 // 包围盒中心
        double y{0.0};
        double z{0.0};
        float bboxMin[3]{0.0f, 0.0f, 0.0f};
        float bboxMax[3]{0.0f, 0.0f, 0.0f};
        Common::String name;
    };

    /// A B-Rep model imported into the data tree.
    ///
    /// Derives MeshData so the existing render, pick, visibility and query pipeline consumes
    /// its tessellated mesh unchanged; the document plus the elemId -> feature table add the
    /// geometry-specific information on top.
    class GeometryData : public MeshData
    {
    public:
        GeometryData(DataNodeId id,
                     const Common::String& name,
                     const Common::SharedPtr<ME::Mesh>& mesh,
                     const Common::SharedPtr<GE::GeometryDocument>& document,
                     const Common::String& sourcePath);

        ~GeometryData() override = default;

        Common::SharedPtr<GE::GeometryDocument> getDocument() const;

        const Common::String& getSourcePath() const;

        /// refs 必须与渲染 Mesh 的 elemId 严格对齐：refs.size() == mesh->getElemNumber()
        /// 且 refs[elemId - 1] 对应 elemId。须在 addDataNode 发布前调用，发布后只读。
        void setElementFeatures(const Common::DynamicArray<GeometryFeatureRef>& refs);

        /// 无效 elemId 返回 GeometryFeatureRef{}（两个 0xFFFFFFFF）。
        GeometryFeatureRef getFeatureByElement(uint32_t elemId) const;

        GeometryFeatureInfo getFeatureInfo(const GeometryFeatureRef& ref) const;

        /// 便捷重载：先按 elemId 反查特征，再填充信息并把 elemId 回填。
        GeometryFeatureInfo getFeatureInfo(uint32_t elemId) const;

        /// 在 Context 数据树中查找持有 mesh 指针值 == meshId 的 GeometryData；找不到返回 null。
        static Common::SharedPtr<GeometryData> findByMesh(uint64_t meshId);

    private:
        mutable std::mutex _geometryMutex;
        Common::SharedPtr<GE::GeometryDocument> _document;    // 构造后不可变
        Common::String _sourcePath;                           // 构造后不可变
        Common::DynamicArray<GeometryFeatureRef> _elementFeatures; // 由 _geometryMutex 保护
    };

} // namespace EgLab::Platform
