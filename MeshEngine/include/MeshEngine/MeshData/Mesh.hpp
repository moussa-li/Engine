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

        /**
         * @brief Adds a node to the mesh.
         *
         * @param node The node to add.
         * @return Common::Return The result of the operation.
         */
        Common::Return addNode(Node &&);

        /**
         * @brief Deletes a node from the mesh by its ID.
         *
         * @return * Common::Return
         */
        Common::Return delNode(IdType);

        /**
         * @brief Adds an element to the mesh.
         *
         * @param elem The element to add.
         * @return Common::Return The result of the operation.
         */
        Common::Return addElem(Elem &&);

        /**
         * @brief Deletes an element from the mesh by its ID.
         *
         * @return * Common::Return
         */
        Common::Return delElem(IdType);

        /**
         * @brief Get the Node Idx object
         *
         * @return * IdxType
         */
        IdxType getNodeIdx(IdType) const;

        /**
         * @brief Get the Elem Idx object
         *
         * @return IdxType
         */
        IdxType getElemIdx(IdType) const;

        /**
         * @brief Get the Elem By Id object
         *
         * @return Elem&
         */
        Elem &getElemById(IdType);

        /**
         * @brief Get the Node By Id object
         *
         * @return Node&
         */
        Node &getNodeById(IdType);

        IdxType getNodeNumber() const;

        IdxType getElemNumber() const;

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