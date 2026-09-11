#pragma once

#include <cstdint>
#include <cstring>

namespace EgLab::ME
{
    struct MeshPOD
    {
        uint32_t nodeNumber{0};
        uint32_t elemNumber{0};

        uint32_t nodeIdsOffset{0};
        uint32_t nodeDataOffset{0};

        uint32_t elemIdsOffset{0};
        uint32_t elemTypesOffset{0};
        uint32_t elemDataOffset{0};

        uint32_t totalDataSize{0};

        uint8_t *data{nullptr};

        MeshPOD() = default;

        MeshPOD(const MeshPOD &other)
        {
            nodeNumber = other.nodeNumber;
            elemNumber = other.elemNumber;
            nodeIdsOffset = other.nodeIdsOffset;
            nodeDataOffset = other.nodeDataOffset;
            elemIdsOffset = other.elemIdsOffset;
            elemTypesOffset = other.elemTypesOffset;
            elemDataOffset = other.elemDataOffset;
            totalDataSize = other.totalDataSize;
            data = nullptr;
            if (other.data != nullptr && other.totalDataSize > 0)
            {
                data = new uint8_t[other.totalDataSize];
                std::memcpy(data, other.data, other.totalDataSize);
            }
        }

        MeshPOD(MeshPOD &&other) noexcept
        {
            nodeNumber = other.nodeNumber;
            elemNumber = other.elemNumber;
            nodeIdsOffset = other.nodeIdsOffset;
            nodeDataOffset = other.nodeDataOffset;
            elemIdsOffset = other.elemIdsOffset;
            elemTypesOffset = other.elemTypesOffset;
            elemDataOffset = other.elemDataOffset;
            totalDataSize = other.totalDataSize;
            data = other.data;

            other.data = nullptr;
            other.totalDataSize = 0;
        }

        MeshPOD &operator=(const MeshPOD &other)
        {
            if (this == &other) return *this;
            clear();
            nodeNumber = other.nodeNumber;
            elemNumber = other.elemNumber;
            nodeIdsOffset = other.nodeIdsOffset;
            nodeDataOffset = other.nodeDataOffset;
            elemIdsOffset = other.elemIdsOffset;
            elemTypesOffset = other.elemTypesOffset;
            elemDataOffset = other.elemDataOffset;
            totalDataSize = other.totalDataSize;
            if (other.data != nullptr && other.totalDataSize > 0)
            {
                data = new uint8_t[other.totalDataSize];
                std::memcpy(data, other.data, other.totalDataSize);
            }
            return *this;
        }

        MeshPOD &operator=(MeshPOD &&other) noexcept
        {
            if (this == &other) return *this;
            clear();
            nodeNumber = other.nodeNumber;
            elemNumber = other.elemNumber;
            nodeIdsOffset = other.nodeIdsOffset;
            nodeDataOffset = other.nodeDataOffset;
            elemIdsOffset = other.elemIdsOffset;
            elemTypesOffset = other.elemTypesOffset;
            elemDataOffset = other.elemDataOffset;
            totalDataSize = other.totalDataSize;
            data = other.data;

            other.data = nullptr;
            other.totalDataSize = 0;
            return *this;
        }

        void clear()
        {
            if (data != nullptr)
            {
                delete[] data;
                data = nullptr;
            }
            totalDataSize = 0;
            nodeNumber = 0;
            elemNumber = 0;
            nodeIdsOffset = nodeDataOffset = elemIdsOffset = elemTypesOffset = elemDataOffset = 0;
        }

        ~MeshPOD()
        {
            clear();
        }
    };
} // namespace EgLab::ME