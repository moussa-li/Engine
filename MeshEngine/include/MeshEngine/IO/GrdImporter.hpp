#pragma once

#include "MeshEngine/IO/Importer.hpp"

namespace EgLab::ME
{
    class MeshEngineAPI GrdImporter : public Importer
    {
    public:
        explicit GrdImporter(const Common::String &fileDir);
        ~GrdImporter() override;

        Common::SharedPtr<Mesh> getMesh(int dim = -1) override;
    };

} // namespace EgLab::ME