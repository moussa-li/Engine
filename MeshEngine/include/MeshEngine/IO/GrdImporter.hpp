#pragma once

#include "MeshEngine/IO/Importer.hpp"

namespace EgLab::ME
{
    class MeshEngineAPI GrdImporter : public Importer
    {
    public:
        explicit GrdImporter(const Common::String &fileDir);
        ~GrdImporter() override;

        Common::SharedPtr<Mesh> getMesh() override;
    };

} // namespace EgLab::ME