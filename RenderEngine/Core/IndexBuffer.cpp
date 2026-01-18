#include "IndexBuffer.hpp"

namespace EgLab
{
    class IndexBuffer::Impl
    {
    };

    IndexBuffer::IndexBuffer(const DynamicArray<IdxType>& indeices) : _impl(new Impl)
    {
    }

} // namespace EgLab