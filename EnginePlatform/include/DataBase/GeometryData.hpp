#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>

#include "Common/DynamicArray.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/String.hpp"
#include "Common/Vector.hpp"
#include "DataBase/MeshData.hpp"
#include "GeometryEngine/Model/GeometryElementMap.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"

namespace EgLab::GE
{
    class GeometryDocument;
}

namespace EgLab::Platform
{
    /// Identifies one feature inside a GeometryData topology.
    using GeometryFeatureRef = GE::FeatureRef;

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

    /// 渲染层中用于高亮一个几何特征的 id 集合：
    /// 面/实体给出 Tri3 单元 id，边给出线段序号，顶点给出渲染节点序号（0 基）。
    struct GeometryFeatureRenderIds
    {
        Common::DynamicArray<uint32_t> elementIds;
        Common::DynamicArray<uint32_t> edgeSegmentIndices;
        Common::DynamicArray<uint32_t> nodeIndices;
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

        /// 重新划分后替换文档（与 setMesh/特征表同步调用）。发布后可调用。
        void setDocument(const Common::SharedPtr<GE::GeometryDocument>& document);

        const Common::String getSourcePath() const;

        /// 更新几何来源路径。由内存拓扑数据（UpdateGeometry）更新后的模型没有可重读的文件来源，
        /// 置空即可让重新划分时跳过该模型而不是回退到旧文件。
        void setSourcePath(const Common::String& sourcePath);

        /// refs 必须与渲染 Mesh 的 elemId 严格对齐：refs.size() == mesh->getElemNumber()
        /// 且 refs[elemId - 1] 对应 elemId。须在 addDataNode 发布前调用，发布后只读。
        void setElementFeatures(const Common::DynamicArray<GeometryFeatureRef>& refs);

        /// 顶点/边的引用表，均以渲染 Mesh 的索引寻址（nodeIdx / 线段序号）。
        /// 须在 addDataNode 发布前调用，发布后只读。
        /// @param nodeFeatures nodeFeatures[nodeIdx] = 该节点渲染的 Vertex 特征；size 需等于节点数。
        /// @param edgeSegmentIndices 线段端点对（每段 2 个 nodeIdx），与渲染线段的顺序一致。
        /// @param edgeSegmentFeatures edgeSegmentFeatures[segment] = 该线段所属的 Edge 特征。
        void setVertexEdgeFeatures(const Common::DynamicArray<GeometryFeatureRef>& nodeFeatures,
                                   const Common::DynamicArray<uint32_t>& edgeSegmentIndices,
                                   const Common::DynamicArray<GeometryFeatureRef>& edgeSegmentFeatures);

        /// 每个渲染节点的曲面 UV（与渲染 Mesh 的节点一一对应，面节点按单个 B-Rep 面归一化到
        /// [0,1]，边/顶点节点为 (0,0)）。须在发布前调用，发布后只读。
        void setNodeUvs(const Common::DynamicArray<Common::Vector2f>& uvs);

        /// 渲染节点的曲面 UV 表；未设置时为空。
        const Common::DynamicArray<Common::Vector2f>& getNodeUvs() const;

        /// 无效 elemId 返回 GeometryFeatureRef{}（两个 0xFFFFFFFF）。
        GeometryFeatureRef getFeatureByElement(uint32_t elemId) const;

        /// 按渲染节点序号查询顶点特征；非顶点节点返回无效引用。
        GeometryFeatureRef getFeatureByNodeIndex(uint32_t nodeIndex) const;

        /// 按渲染节点的节点 id 查询顶点特征。
        GeometryFeatureRef getFeatureByNodeId(uint32_t nodeId) const;

        /// 按渲染线段序号查询边特征；无该线段时返回无效引用。
        GeometryFeatureRef getFeatureByEdgeSegment(uint32_t segmentIndex) const;

        /// 该节点是否渲染为几何顶点（即 ref 到 Vertex 特征）。
        bool isVertexNode(uint32_t nodeIndex) const;

        /// 线段端点对（nodeIdx），供渲染与拾取共用。
        const Common::DynamicArray<uint32_t>& getEdgeSegmentIndices() const;

        /// 是否存在可直接渲染的 B-Rep 边线段。
        bool hasBrepEdges() const;

        /// 特征 -> 渲染单元（elemId）映射；特征级拾取/框选使用。发布后只读。
        const GE::GeometryElementMap& getElementMap() const;

        /// 返回 ref 的指定类型的父特征（沿拓扑层级向上查找）；无父特征时返回无效引用。
        GeometryFeatureRef findParentFeature(const GeometryFeatureRef& ref,
                                             GE::TopologyType parentType) const;

        /// 按特征类型掩码过滤 ref：
        /// - ref 的类型被允许时原样返回；
        /// - Face 被禁用但 Solid 被允许时，上溯到所属 Solid 并返回该 Solid；
        /// - 其余情况返回无效引用。
        GeometryFeatureRef resolveByMask(const GeometryFeatureRef& ref, uint32_t mask) const;

        GeometryFeatureInfo getFeatureInfo(const GeometryFeatureRef& ref) const;

        /// 便捷重载：先按 elemId 反查特征，再填充信息并把 elemId 回填。
        GeometryFeatureInfo getFeatureInfo(uint32_t elemId) const;

        /// 按类型 + 索引获取特征信息（等价于 getFeatureInfo(FeatureRef(type, index))）。
        GeometryFeatureInfo getFeatureInfo(GE::TopologyType type, GE::IdxType index) const;

        /// 某一拓扑类型的特征数量；文档缺失时返回 0。
        std::size_t getFeatureCount(GE::TopologyType type) const;

        /// 获取 ref 的直接父特征与子特征引用（父特征高一个维度，子特征低一个维度）。
        void getFeatureRelations(const GeometryFeatureRef& ref,
                                 Common::DynamicArray<GeometryFeatureRef>& parents,
                                 Common::DynamicArray<GeometryFeatureRef>& children) const;

        /// 收集渲染 ref 所需的 id（面单元 / 边线段 / 顶点渲染节点）。
        void collectFeatureRenderIds(const GeometryFeatureRef& ref,
                                     GeometryFeatureRenderIds& out) const;

        /// 在 Context 数据树中查找持有 mesh 指针值 == meshId 的 GeometryData；找不到返回 null。
        static Common::SharedPtr<GeometryData> findByMesh(uint64_t meshId);

    private:
        mutable std::mutex _geometryMutex;
        Common::SharedPtr<GE::GeometryDocument> _document;    // 由 _geometryMutex 保护
        Common::String _sourcePath;                           // 由 _geometryMutex 保护
        Common::DynamicArray<GeometryFeatureRef> _elementFeatures; // 由 _geometryMutex 保护
        GE::GeometryElementMap _elementMap;                        // 由 _geometryMutex 保护
        Common::DynamicArray<GeometryFeatureRef> _nodeFeatures;    // 由 _geometryMutex 保护
        Common::DynamicArray<Common::Vector2f> _nodeUvs;           // 由 _geometryMutex 保护
        Common::DynamicArray<uint32_t> _edgeSegmentIndices;        // 由 _geometryMutex 保护
        Common::DynamicArray<GeometryFeatureRef> _edgeSegmentFeatures; // 由 _geometryMutex 保护
    };

} // namespace EgLab::Platform
