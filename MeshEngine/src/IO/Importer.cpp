#include "MeshEngine/IO/Importer.hpp"

#include <filesystem>

#include "MeshEngine/IO/IOFormat.hpp"

namespace EgLab::ME
{
    class Importer::Impl
    {
    public:
        virtual ~Impl()
        {
        }
        Common::String _fileDir;
        Suffix _suffix;
    };

    Importer::Importer(const Common::String &fileDir) : _impl(Common::makeUnique<Impl>())
    {
        std::filesystem::path filePath = fileDir.c_str();
        auto extension = filePath.extension();
        // TOOD: maybe implacement by factory is better

        if (extension == ".msh")
        {
            _impl->_suffix = Suffix::msh;
        }
        _impl->_fileDir = fileDir;
    }

    Importer::~Importer()
    {
    }

    const Common::String &Importer::getFileDir() const
    {
        return _impl->_fileDir;
    }
} // namespace EgLab::ME