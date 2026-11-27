#include "OcctGeometryImporter.hpp"

#include <cstring>
#include <fstream>
#include <istream>
#include <streambuf>

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
        /// Hooks a std::istream onto caller-provided bytes without copying them.
        class MemoryStreamBuffer : public std::streambuf
        {
        public:
            MemoryStreamBuffer(const char* data, std::size_t size)
            {
                char* first = const_cast<char*>(data);
                setg(first, first, first + size);
            }
        };

        /// Converts one native shape into the neutral document. Error reporting stays with the
        /// caller so file and buffer imports share the same conversion path.
        void convertShape(const TopoDS_Shape& shape, const GeometryImportOptions& options,
                          const Common::String& kernelName, GeometryDocument& document,
                          GeometryImportReport& report)
        {
            OcctShapeConverter converter(options);
            const Common::Return status = converter.convert(shape, document, report);
            report.setKernelName(kernelName);
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

        /// Both BREP encodings start with a human readable banner, so the first bytes tell
        /// which reader to use without trying the wrong one on large payloads.
        enum class BrepEncoding
        {
            Binary,
            Ascii,
            Unknown
        };

        bool containsToken(const char* data, std::size_t scanLength, const char* token)
        {
            const std::size_t tokenLength = std::strlen(token);
            for (std::size_t offset = 0; offset + tokenLength <= scanLength; ++offset)
            {
                if (std::memcmp(data + offset, token, tokenLength) == 0)
                {
                    return true;
                }
            }
            return false;
        }

        BrepEncoding detectBrepEncoding(const char* data, std::size_t size)
        {
            const std::size_t scanLength = size < 64 ? size : 64;
            if (containsToken(data, scanLength, "Open CASCADE Topology"))
            {
                return BrepEncoding::Binary;
            }
            if (containsToken(data, scanLength, "CASCADE Topology V1"))
            {
                return BrepEncoding::Ascii;
            }
            return BrepEncoding::Unknown;
        }

        /// Reads a shape from bytes produced by BinTools::Write or BRepTools::Write.
        bool readFromBuffer(const char* data, std::size_t size, TopoDS_Shape& shape,
                            Common::String& error)
        {
            const BrepEncoding encoding = detectBrepEncoding(data, size);
            if (encoding != BrepEncoding::Ascii)
            {
                try
                {
                    MemoryStreamBuffer buffer(data, size);
                    std::istream stream(&buffer);
                    BinTools::Read(shape, stream);
                    if (!shape.IsNull())
                    {
                        return true;
                    }
                }
                catch (const Standard_Failure&)
                {
                    shape.Nullify();
                }
            }

            if (encoding != BrepEncoding::Binary)
            {
                try
                {
                    MemoryStreamBuffer buffer(data, size);
                    std::istream stream(&buffer);
                    BRep_Builder builder;
                    BRepTools::Read(shape, stream, builder);
                    if (!shape.IsNull())
                    {
                        return true;
                    }
                }
                catch (const Standard_Failure&)
                {
                    shape.Nullify();
                }
            }

            error = Common::String("OCCT failed to read the serialized BREP data");
            return false;
        }
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

            convertShape(shape, _options, getName(), document, report);
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

    GeometryImportReport OcctGeometryImporter::importBuffer(const void* data, std::size_t size,
                                                            GeometryDocument& document) const
    {
        GeometryImportReport report;
        report.setKernelName(getName());

        if (data == nullptr || size == 0)
        {
            report.setStatus(Common::Return::BadInput);
            report.setMessage(Common::String("the serialized geometry buffer is empty"));
            return report;
        }

        document.clear();

        try
        {
            TopoDS_Shape shape;
            Common::String error;
            if (!readFromBuffer(static_cast<const char*>(data), size, shape, error))
            {
                report.setStatus(Common::Return::Failed);
                report.setMessage(error);
                document.clear();
                return report;
            }

            convertShape(shape, _options, getName(), document, report);
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
