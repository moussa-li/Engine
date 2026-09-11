#pragma once

#include "MeshEngine/IO/Importer.hpp"

namespace EgLab::ME
{
    class MeshEngineAPI GmshImporter : public Importer
    {
    public:
        GmshImporter(const Common::String &fileDir);
        virtual ~GmshImporter();

        virtual Common::SharedPtr<Mesh> getMesh() override;

    private:
        class Impl;

        Common::UniquePtr<Impl> _impl;
    };

} // namespace EgLab::ME
