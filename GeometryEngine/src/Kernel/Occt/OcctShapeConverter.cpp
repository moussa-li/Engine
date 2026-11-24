#include "OcctShapeConverter.hpp"

#include <BRepAdaptor_Curve.hxx>
#include <BRepBndLib.hxx>
#include <BRepGProp.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <GCPnts_QuasiUniformDeflection.hxx>
#include <GProp_GProps.hxx>
#include <Poly_Triangle.hxx>
#include <Poly_Triangulation.hxx>
#include <Standard_Failure.hxx>
#include <Standard_Integer.hxx>
#include <TopAbs_Orientation.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Vertex.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>

#include "Common/DynamicArray.hpp"
#include "Common/String.hpp"
#include "Common/StringTools.hpp"
#include "GeometryEngine/Model/GeometryDocument.hpp"

namespace EgLab::GE
{
    class OcctShapeConverter::Impl
    {
    public:
        explicit Impl(const GeometryImportOptions& options) : _options(options)
        {
        }

        Common::Return convert(const TopoDS_Shape& shape, GeometryDocument& document,
                               GeometryImportReport& report)
        {
            _triangleCount = 0;
            _solids.Clear();
            _faces.Clear();
            _edges.Clear();
            _vertices.Clear();

            BRepMesh_IncrementalMesh mesher(
                shape, _options.getLinearDeflection(),
                _options.isRelativeDeflection() ? Standard_True : Standard_False,
                _options.getAngularDeflection(),
                _options.isParallel() ? Standard_True : Standard_False);
            if (!mesher.IsDone())
            {
                report.setStatus(Common::Return::Failed);
                report.setMessage(Common::String("OCCT failed to tessellate the imported shape"));
                return Common::Return::Failed;
            }

            TopExp::MapShapes(shape, TopAbs_SOLID, _solids);
            TopExp::MapShapes(shape, TopAbs_FACE, _faces);
            TopExp::MapShapes(shape, TopAbs_EDGE, _edges);
            TopExp::MapShapes(shape, TopAbs_VERTEX, _vertices);

            buildVertices(document);
            buildEdges(document);
            buildFaces(document);
            buildSolids(document);
            linkRelations(document);

            report.setFeatureCount(TopologyType::Vertex, count(_vertices));
            report.setFeatureCount(TopologyType::Edge, count(_edges));
            report.setFeatureCount(TopologyType::Face, count(_faces));
            report.setFeatureCount(TopologyType::Solid, count(_solids));
            report.setTriangleCount(_triangleCount);
            report.setStatus(Common::Return::Succeed);

            if (_faces.Extent() == 0 && _solids.Extent() == 0)
            {
                report.setMessage(Common::String("the file contains no face or solid"));
            }
            return Common::Return::Succeed;
        }

    private:
        static std::size_t count(const TopTools_IndexedMapOfShape& shapes)
        {
            return static_cast<std::size_t>(shapes.Extent());
        }

        Common::String makeFeatureName(const char* prefix, Standard_Integer index) const
        {
            Common::String name(prefix);
            name.append("_");
            name.append(Common::ToString(static_cast<int>(index)).c_str());
            return name;
        }

        void setBoundingBox(const TopoDS_Shape& shape, Geometry& geometry) const
        {
            Bnd_Box box;
            BRepBndLib::Add(shape, box, Standard_False);
            if (box.IsVoid()) return;

            Standard_Real xMin = 0.0;
            Standard_Real yMin = 0.0;
            Standard_Real zMin = 0.0;
            Standard_Real xMax = 0.0;
            Standard_Real yMax = 0.0;
            Standard_Real zMax = 0.0;
            box.Get(xMin, yMin, zMin, xMax, yMax, zMax);

            BoundingBox boundingBox;
            boundingBox.addPoint(Vec3(xMin, yMin, zMin));
            boundingBox.addPoint(Vec3(xMax, yMax, zMax));
            geometry.setBoundingBox(boundingBox);
        }

        void buildVertices(GeometryDocument& document)
        {
            for (Standard_Integer i = 1; i <= _vertices.Extent(); ++i)
            {
                const TopoDS_Vertex& vertex = TopoDS::Vertex(_vertices(i));
                const IdxType index =
                    document.createFeature(TopologyType::Vertex, makeFeatureName("Vertex", i));
                if (index == INVALID_IDX) continue;

                Geometry* geometry = document.getGeometry(TopologyType::Vertex, index);
                if (geometry == nullptr) continue;

                const gp_Pnt point = BRep_Tool::Pnt(vertex);
                const Vec3 position(point.X(), point.Y(), point.Z());

                Tessellation& mesh = geometry->getTessellation();
                const IdxType nodeIndex = mesh.addNode(position);
                mesh.addElement(&nodeIndex, 1);

                BoundingBox bound;
                bound.addPoint(position);
                geometry->setBoundingBox(bound);
            }
        }

        void buildEdges(GeometryDocument& document)
        {
            for (Standard_Integer i = 1; i <= _edges.Extent(); ++i)
            {
                const TopoDS_Edge& edge = TopoDS::Edge(_edges(i));
                const IdxType index =
                    document.createFeature(TopologyType::Edge, makeFeatureName("Edge", i));
                if (index == INVALID_IDX) continue;

                Geometry* geometry = document.getGeometry(TopologyType::Edge, index);
                if (geometry == nullptr) continue;

                GProp_GProps properties;
                BRepGProp::LinearProperties(edge, properties);
                geometry->setLength(properties.Mass());

                Tessellation& mesh = geometry->getTessellation();
                Common::DynamicArray<Vec3> points;

                try
                {
                    BRepAdaptor_Curve curve(edge);
                    GCPnts_QuasiUniformDeflection sampler(curve, _options.getLinearDeflection());
                    if (sampler.IsDone() && sampler.NbPoints() >= 2)
                    {
                        points.reserve(static_cast<std::size_t>(sampler.NbPoints()));
                        for (Standard_Integer p = 1; p <= sampler.NbPoints(); ++p)
                        {
                            const gp_Pnt point = sampler.Value(p);
                            points.pushBack(Vec3(point.X(), point.Y(), point.Z()));
                        }
                    }
                }
                catch (const Standard_Failure&)
                {
                    // Degenerate or invalid edges fall back to their end vertices below.
                    points.clear();
                }

                if (points.size() < 2)
                {
                    TopoDS_Vertex firstVertex;
                    TopoDS_Vertex lastVertex;
                    TopExp::Vertices(edge, firstVertex, lastVertex);
                    if (!firstVertex.IsNull())
                    {
                        const gp_Pnt point = BRep_Tool::Pnt(firstVertex);
                        points.pushBack(Vec3(point.X(), point.Y(), point.Z()));
                    }
                    if (!lastVertex.IsNull())
                    {
                        const gp_Pnt point = BRep_Tool::Pnt(lastVertex);
                        points.pushBack(Vec3(point.X(), point.Y(), point.Z()));
                    }
                }

                Common::DynamicArray<IdxType> polyline;
                polyline.reserve(points.size());
                for (std::size_t p = 0; p < points.size(); ++p)
                {
                    polyline.pushBack(mesh.addNode(points[p]));
                }
                if (polyline.size() >= 2)
                {
                    mesh.addElement(polyline.data(), polyline.size());
                }
                setBoundingBox(edge, *geometry);
            }
        }

        void buildFaces(GeometryDocument& document)
        {
            for (Standard_Integer i = 1; i <= _faces.Extent(); ++i)
            {
                const TopoDS_Face& face = TopoDS::Face(_faces(i));
                const IdxType index =
                    document.createFeature(TopologyType::Face, makeFeatureName("Face", i));
                if (index == INVALID_IDX) continue;

                Geometry* geometry = document.getGeometry(TopologyType::Face, index);
                if (geometry == nullptr) continue;

                GProp_GProps properties;
                BRepGProp::SurfaceProperties(face, properties);
                geometry->setArea(properties.Mass());

                TopLoc_Location location;
                Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);
                if (!triangulation.IsNull())
                {
                    Tessellation& mesh = geometry->getTessellation();
                    mesh.reserveNodes(static_cast<std::size_t>(triangulation->NbNodes()));

                    const gp_Trsf& transform = location.Transformation();
                    for (Standard_Integer n = 1; n <= triangulation->NbNodes(); ++n)
                    {
                        const gp_Pnt point = triangulation->Node(n).Transformed(transform);
                        mesh.addNode(Vec3(point.X(), point.Y(), point.Z()));
                    }

                    const bool reversed = face.Orientation() == TopAbs_REVERSED;
                    for (Standard_Integer t = 1; t <= triangulation->NbTriangles(); ++t)
                    {
                        Standard_Integer first = 0;
                        Standard_Integer second = 0;
                        Standard_Integer third = 0;
                        triangulation->Triangle(t).Get(first, second, third);
                        if (reversed)
                        {
                            const Standard_Integer swapValue = second;
                            second = third;
                            third = swapValue;
                        }
                        mesh.addTriangle(static_cast<IdxType>(first - 1),
                                         static_cast<IdxType>(second - 1),
                                         static_cast<IdxType>(third - 1));
                    }
                    _triangleCount += static_cast<std::size_t>(triangulation->NbTriangles());
                }

                setBoundingBox(face, *geometry);
            }
        }

        void buildSolids(GeometryDocument& document)
        {
            for (Standard_Integer i = 1; i <= _solids.Extent(); ++i)
            {
                const TopoDS_Solid& solid = TopoDS::Solid(_solids(i));
                const IdxType index =
                    document.createFeature(TopologyType::Solid, makeFeatureName("Solid", i));
                if (index == INVALID_IDX) continue;

                Geometry* geometry = document.getGeometry(TopologyType::Solid, index);
                if (geometry == nullptr) continue;

                GProp_GProps properties;
                BRepGProp::VolumeProperties(solid, properties);
                geometry->setVolume(properties.Mass());

                Tessellation& mesh = geometry->getTessellation();
                for (TopExp_Explorer explorer(solid, TopAbs_FACE); explorer.More(); explorer.Next())
                {
                    const Standard_Integer faceIndex = _faces.FindIndex(explorer.Current());
                    if (faceIndex <= 0) continue;

                    const Geometry* faceGeometry =
                        document.getGeometry(TopologyType::Face,
                                             static_cast<IdxType>(faceIndex - 1));
                    if (faceGeometry == nullptr) continue;
                    mesh.append(faceGeometry->getTessellation());
                }

                setBoundingBox(solid, *geometry);
            }
        }

        void linkRelations(GeometryDocument& document)
        {
            for (Standard_Integer i = 1; i <= _solids.Extent(); ++i)
            {
                const TopoDS_Solid& solid = TopoDS::Solid(_solids(i));
                for (TopExp_Explorer explorer(solid, TopAbs_FACE); explorer.More(); explorer.Next())
                {
                    const Standard_Integer child = _faces.FindIndex(explorer.Current());
                    if (child <= 0) continue;
                    document.linkFeatures(TopologyType::Solid, static_cast<IdxType>(i - 1),
                                          TopologyType::Face, static_cast<IdxType>(child - 1));
                }
            }

            for (Standard_Integer i = 1; i <= _faces.Extent(); ++i)
            {
                const TopoDS_Face& face = TopoDS::Face(_faces(i));
                for (TopExp_Explorer explorer(face, TopAbs_EDGE); explorer.More(); explorer.Next())
                {
                    const Standard_Integer child = _edges.FindIndex(explorer.Current());
                    if (child <= 0) continue;
                    document.linkFeatures(TopologyType::Face, static_cast<IdxType>(i - 1),
                                          TopologyType::Edge, static_cast<IdxType>(child - 1));
                }
            }

            for (Standard_Integer i = 1; i <= _edges.Extent(); ++i)
            {
                const TopoDS_Edge& edge = TopoDS::Edge(_edges(i));
                for (TopExp_Explorer explorer(edge, TopAbs_VERTEX); explorer.More(); explorer.Next())
                {
                    const Standard_Integer child = _vertices.FindIndex(explorer.Current());
                    if (child <= 0) continue;
                    document.linkFeatures(TopologyType::Edge, static_cast<IdxType>(i - 1),
                                          TopologyType::Vertex, static_cast<IdxType>(child - 1));
                }
            }
        }

        GeometryImportOptions _options;
        TopTools_IndexedMapOfShape _solids;
        TopTools_IndexedMapOfShape _faces;
        TopTools_IndexedMapOfShape _edges;
        TopTools_IndexedMapOfShape _vertices;
        std::size_t _triangleCount{0};
    };

    OcctShapeConverter::OcctShapeConverter(const GeometryImportOptions& options)
        : _impl(Common::makeUnique<Impl>(options))
    {
    }

    OcctShapeConverter::~OcctShapeConverter() = default;

    Common::Return OcctShapeConverter::convert(const TopoDS_Shape& shape, GeometryDocument& document,
                                               GeometryImportReport& report)
    {
        return _impl->convert(shape, document, report);
    }

} // namespace EgLab::GE
