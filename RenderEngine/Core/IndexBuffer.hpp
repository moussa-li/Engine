#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Object.hpp"
#include "Definites.hpp"

namespace EgLab
{
    class IndexBuffer : public Object
    {
    public:
        IndexBuffer(const DynamicArray<IdxType>& indeices);

    private:
        class Impl;
        Impl* _impl;
    };

} // namespace EgLab