/**
 * @file UniquePtr.hpp
 * @author Moussa-Li
 * @brief just like std::unique_ptr
 * @date 2025-10-26
 */

#pragma once

#include "Common/CommonAPI.hpp"
#include "Common/PtrBase.hpp"
#include "Common/Utils.hpp"

namespace EgLab
{

    template <class T>
    class UniquePtr : public PtrBase<T>
    {
    public:
        UniquePtr()
        {
            this->_ptr = nullptr;
        }

        UniquePtr(T *ptr)
        {
            this->_ptr = ptr;
        }

        ~UniquePtr()
        {
            if (this->_ptr == nullptr) return;

            delete this->_ptr;
        }
    };

    template <class T, class... Args>
    UniquePtr<T> makeUnique(Args &&...args)
    {
        return UniquePtr<T>(new T(forward<Args>(args)...));
    }

    template <class Derived, class Base>
    UniquePtr<Derived> staticUniquePtrCast(UniquePtr<Base> &&ptr)
    {
        return UniquePtr<Derived>(static_cast<Derived *>(ptr.release()));
    }

    template <class Derived, typename Base>
    UniquePtr<Derived> dynamicUniquePtrCast(UniquePtr<Base> &&ptr)
    {
        Derived *derivedPtr = dynamic_cast<Derived *>(ptr.get());
        if (derivedPtr)
        {
            ptr.release();
            return UniquePtr<Derived>(derivedPtr);
        }
        return UniquePtr<Derived>(nullptr);
    }

} // namespace EgLab