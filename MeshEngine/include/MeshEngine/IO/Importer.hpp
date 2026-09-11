#pragma once

#include "Common/SharedPtr.hpp"
#include "Common/String.hpp"
#include "Common/UniquePtr.hpp"
#include "MeshEngine/Common/MeshEngineAPI.hpp"

namespace EgLab::ME
{
    class Mesh;
    class ImporterFactory;
    class MeshEngineAPI Importer
    {
    public:
        Importer(const Common::String &fileDir);
        virtual ~Importer();

        const Common::String &getFileDir() const;

        virtual Common::SharedPtr<Mesh> getMesh() = 0;

    private:
        class Impl;
        Common::UniquePtr<Impl> _impl;
    };
} // namespace EgLab::ME