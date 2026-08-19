#pragma once
#include "Common/DynamicArray.hpp"
#include "Core/Definites.hpp"
#include "RenderEngine/Core/RenderEngineAPI.hpp"
#include "RenderEngine/Core/RenderPrimitive.hpp"
// #include "RenderEngine/Core/VertexArray.hpp"

namespace EgLab::RE
{
    class Shader;
    class Transform;
    class Texture;
    class VertexArray;
    class VertexBuffer;
    class ShaderStorageBuffer;

    class RenderEngineAPI RenderNode : public RenderPrimitive
    {
    public:
        virtual ~RenderNode();
        virtual void setup() override;
        virtual void draw(Common::SharedPtr<Shader> shader) override;

        void setNodes(Common::DynamicArray<CoordType>&&);

    private:
        Common::DynamicArray<CoordType> _vertices;
        Common::SharedPtr<VertexArray> _vertexArray;
        Common::SharedPtr<VertexBuffer> _vertexBuffer;
    };
} // namespace EgLab::RE