#pragma once
#include "MeshEngine/MeshData/Mesh.hpp"
#include "RenderEngine/Core/PrimitiveCreator.hpp"

namespace EgLab::RE
{

    class MeshPrimitiveCreator : public PrimitiveCreator
    {
    public:
        MeshPrimitiveCreator(ME::Mesh&);

    protected:
        virtual void updateData(Common::SharedPtr<RenderNode>);
        virtual void updateData(Common::SharedPtr<RenderLine>);
        virtual void updateData(Common::SharedPtr<RenderFace>);

    private:
        ME::Mesh& _mesh;
    };
} // namespace EgLab::RE