#pragma once
#include "MeshEngine/MeshData/Mesh.hpp"
#include "RenderEngine/Core/MeshToTriFace.hpp"
#include "RenderEngine/Core/PrimitiveCreator.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"

namespace EgLab::RE
{

    class RenderEngineAPI MeshPrimitiveCreator : public PrimitiveCreator
    {
    public:
        MeshPrimitiveCreator(Common::SharedPtr<ME::Mesh>);

        ~MeshPrimitiveCreator();

    protected:
        virtual void updateData(Common::SharedPtr<RenderNode>);
        virtual void updateData(Common::SharedPtr<RenderLine>);
        virtual void updateData(Common::SharedPtr<RenderFace>);

    private:
        Common::SharedPtr<ME::Mesh> _mesh;
        MeshToTriFace _translator;
    };
} // namespace EgLab::RE