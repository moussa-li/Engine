#include "OcctGeometryImporter.hpp"

#include <cstring>
#include <fstream>

#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <BinTools.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <STEPControl_Reader.hxx>
#include <Standard_Failure.hxx>
#include <TopoDS_Shape.hxx>

#include "Common/String.hpp"
#include "GeometryEngine/Model/GeometryDocument.hpp"
#include "OcctShapeConverter.hpp"

namespace EgLab::GE
{
    namespace
    {
        /// Reads STEP or BREP files and hands the native shape to the converter.
        class OcctFileReader
        {
        public:
            bool read(const Common::String& filePath, TopoDS_Shape& shape,
                      Common::String& error) const
            {
                if (hasExtension(filePath, ".stp") || hasExtension(filePath, ".step"))
                {
                    return readStep(filePath, shape, error);
                }

                if (hasExtension(filePath, ".brp") || hasExtension(filePath, ".brep"))
                {
                    return readBrep(filePath, shape, error);
                }

                if (readBrep(filePath, shape, error)) return true;
                error = Common::String();
                return readStep(filePath, shape, error);
            }

        private:
            static bool hasExtension(const Common::String& path, const char* extension)
            {
                const std::size_t extensionLength = std::strlen(extension);
                if (path.size() < extensionLength) return false;

                const char* tail = path.c_str() + (path.size() - extensionLength);
                for (std::size_t i = 0; i < extensionLength; ++i)
                {
                    char actual = tail[i];
                    char expected = extension[i];
                    if (actual >= 'A' && actual <= 'Z') actual = static_cast<char>(actual - 'A' + 'a');
                    if (expected >= 'A' && expected <= 'Z') expected = static_cast<char>(expected - 'A' + 'a');
                    if (actual != expected) return false;
                }
                return true;
            }

            static bool readStep(const Common::String& filePath, TopoDS_Shape& shape,
                                 Common::String& error)
            {
                STEPControl_Reader reader;
                const IFSelect_ReturnStatus status = reader.ReadFile(filePath.c_str());
                if (status != IFSelect_RetDone)
                {
                    error = Common::String("OCCT failed to read the STEP file");
                    return false;
                }

                reader.TransferRoots();
                shape = reader.OneShape();
                if (shape.IsNull())
                {
                    error = Common::String("the STEP file does not contain a transferable shape");
                    return false;
                }
                return true;
            }

            static bool readBrep(const Common::String& filePath, TopoDS_Shape& shape,
                                 Common::String& error)
            {
                BRep_Builder builder;
                if (BRepTools::Read(shape, filePath.c_str(), builder) && !shape.IsNull())
                {
                    return true;
                }

                // Binary BREP files are produced by BinTools; try them when the ASCII reader fails.
                if (BinTools::Read(shape, filePath.c_str()) && !shape.IsNull())
                {
                    return true;
                }

                error = Common::String("OCCT failed to read the BREP file");
                return false;
            }
        };
    } // namespace

    OcctGeometryImporter::OcctGeometryImporter(const GeometryImportOptions& options)
        : _options(options)
    {
    }

    Common::String OcctGeometryImporter::getName() const
    {
        return Common::String("OCCT");
    }

    GeometryImportReport OcctGeometryImporter::importFile(const Common::String& filePath,
                                                          GeometryDocument& document) const
    {
        GeometryImportReport report;
        report.setFilePath(filePath);
        report.setKernelName(getName());

        std::ifstream probe(filePath.c_str(), std::ios::binary);
        if (!probe.good())
        {
            report.setStatus(Common::Return::FileNotFound);
            report.setMessage(Common::String("geometry file not found"));
            return report;
        }
        probe.close();

        document.clear();

        try
        {
            TopoDS_Shape shape;
            Common::String error;
            OcctFileReader reader;
            if (!reader.read(filePath, shape, error))
            {
                report.setStatus(Common::Return::Failed);
                report.setMessage(error);
                document.clear();
                return report;
            }

            OcctShapeConverter converter(_options);
            const Common::Return status = converter.convert(shape, document, report);
            report.setKernelName(getName());
            if (status != Common::Return::Succeed)
            {
                report.setStatus(status);
                if (report.getMessage().size() == 0)
                {
                    report.setMessage(Common::String("OCCT failed to convert the imported shape"));
                }
                document.clear();
            }
        }
        catch (const Standard_Failure& failure)
        {
            const char* description = failure.GetMessageString();
            report.setStatus(Common::Return::Failed);
            report.setMessage(Common::String("OCCT failure: ") +
                              Common::String(description != nullptr ? description : "unknown"));
            document.clear();
        }
        catch (...)
        {
            report.setStatus(Common::Return::Failed);
            report.setMessage(Common::String("unknown failure raised by OCCT"));
            document.clear();
        }

        return report;
    }

} // namespace EgLab::GE
