#pragma once
#include "Common/SharedPtr.hpp"
#include "Common/Vector.hpp"
#include "DataBase/DataTree.hpp"
#include "MeshEngine/MeshData/Mesh.hpp"


namespace EgLab::Platform
{
    class MeshData : public DataTree
    {
    public:
        MeshData(const Common::String& name, const Common::SharedPtr<ME::Mesh>& mesh);

        const Common::SharedPtr<ME::Mesh>& getMesh() const;
        const Common::Vector4f& getColor() const;
        void setColor(const Common::Vector4f& color);

    private:
        Common::SharedPtr<ME::Mesh> _mesh;
        Common::Vector4f _color{0.8f, 0.8f, 0.8f, 1.0f};
    };
} // namespace EgLab::Platform