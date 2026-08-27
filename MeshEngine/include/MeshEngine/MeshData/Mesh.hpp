#pragma once
#include "Common/UniquePtr.hpp"
#include "MeshEngine/MeshData/Element.hpp"
#include "MeshEngine/MeshData/Node.hpp"

namespace EgLab::ME
{
    class Mesh
    {
    public:
        Mesh();

        ~Mesh();

        Common::Return addNode(Node &&);

        Common::Return delNode(IdType);

        Common::Return addElem(Elem &&);

        Common::Return delElem(IdType);

        IdxType getNodeIdx(IdType) const;

        IdxType getElemIdx(IdType) const;

        Elem &getElemById(IdType);

        Node &getNodeById(IdType);

    private:
        class Impl;
        Common::UniquePtr<Impl> _impl;
        friend class MeshIterator;
    };

    class MeshIterator
    {
    public:
        MeshIterator(Mesh &);
        ~MeshIterator();

        const Node &currentNode() const;

        Node &currentNode();

        bool nextNode();

        const Elem &currentElem() const;

        Elem &currentElem();

        bool nextElem();

    protected:
        class Impl;
        Common::UniquePtr<Impl> _impl;
    };

} // namespace EgLab::ME