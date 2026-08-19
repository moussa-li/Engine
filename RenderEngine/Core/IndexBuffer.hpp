#pragma once

#include "Common/DynamicArray.hpp"
#include "Common/Object.hpp"
#include "Definites.hpp"

namespace EgLab::RE
{
    class IndexBuffer
    {
    public:
        IndexBuffer(const Common::DynamicArray<IdxType>& indeices);
        ~IndexBuffer();

        void bind() const;
        void unBind() const;

        unsigned int getCount() const;

    private:
        class Impl;
        Impl* _impl;
    };

} // namespace EgLab::RE