#pragma once
#include <cstddef>

namespace EgLab::Common
{
    template <typename T>
    class SharedPtr;

    class Object
    {
    public:
        inline size_t getRef()
        {
            return ref;
        }

    private:
        inline void addRef()
        {
            ref++;
        }
        inline void subRef()
        {
            ref--;
        }

        size_t ref{0}; // for shared ptr
        template <typename T>
        friend class SharedPtr;
    };

} // namespace EgLab::Common