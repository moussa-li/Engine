#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Return.hpp"
#include "Common/UniquePtr.hpp"
#include "MeshEngine/Common/Definitions.hpp"
#include "MeshEngine/MeshData/HEFace.hpp"
#include "MeshEngine/MeshData/HENode.hpp"
#include "MeshEngine/MeshData/HalfEdge.hpp"

namespace EgLab::ME
{

    class HEMesh
    {
    public:
        HEMesh();

        ~HEMesh();

        IdType createVertex(float x, float y, float z);

        Common::Return createVertex(IdType id, float x, float y, float z);

        Common::Return createFace(const Common::DynamicArray<IdType>& vertexIds);

        std::size_t getVertexCount() const;

        std::size_t getFaceCount() const;

        std::size_t getHalfEdgeCount() const;

        HENode getVertex(IdxType index) const;

        IdxType getVertexIndex(IdType id) const;

        HEFace getFace(IdxType index) const;

        HalfEdge getHalfEdge(IdxType index) const;

    private:
        class Impl;
        Common::UniquePtr<Impl> _impl;
    };
} // namespace EgLab::ME