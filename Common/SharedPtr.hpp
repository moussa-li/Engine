#pragma once
/**
 * @file SharedPtr.hpp
 * @author Moussa-Li
 * @brief The usage is the same as shared ptr,
 *  but it is an embedded smart pointer,
 *  so you need to let T drived from Object
 * @date 2026-01-18
 */

#include "Common/CommonAPI.hpp"
#include "Common/PtrBase.hpp"
#include "Common/Utils.hpp"

namespace EgLab
{
    template <typename T, typename = void>
    struct HasRefMember : FalseType
    {
    };

    template <typename T>
    struct HasRefMember<T, void_t<decltype(declval<T>().ref)>> : TrueType
    {
    };

    template <typename T>
    class SharedPtr : public PtrBase<T>
    {
        // static_assert(HasRefMember<T>::value, "T has member ref");

    public:
        SharedPtr()
        {
            this->_ptr = nullptr;
        }

        ~SharedPtr()
        {
            if (this->_ptr)
            {
                this->_ptr->subRef();
                if (this->_ptr->getRef() == 0)
                {
                    delete this->_ptr;
                    this->_ptr = nullptr;
                }
            }
        }

        explicit SharedPtr(T *ptr)
        {
            this->_ptr = ptr;
            if (this->_ptr)
            {
                ptr->addRef();
            }
        }

        SharedPtr(const SharedPtr &other)
        {
            this->_ptr = other._ptr;
            if (this->_ptr)
            {
                this->_ptr->addRef();
            }
        }

        void operator=(T *ptr)
        {
            T *lastPtr = this->_ptr;
            this->_ptr = ptr;
            if (this->_ptr)
            {
                ptr->addRef();
            }
            if (lastPtr)
            {
                lastPtr->subRef();
                if (lastPtr->getRef() == 0)
                {
                    delete lastPtr;
                    lastPtr = nullptr;
                }
            }
        }

        SharedPtr<T> &operator=(const SharedPtr<T> &other)
        {
            this->operator=(other.get());
            return *this;
        }

        SharedPtr<T> &operator=(SharedPtr<T> &&other)
        {
            this->operator=(other.get());
            return *this;
        }

    private:
    };

    template <class T, class... Args>
    SharedPtr<T> makeShared(Args &&...args)
    {
        return SharedPtr<T>(new T(forward<Args>(args)...));
    }

    template <class Derived, typename Base>
    SharedPtr<Derived> dynamicSharedPtrCast(SharedPtr<Base> &&ptr)
    {
        Derived *derivedPtr = dynamic_cast<Derived *>(ptr.get());
        if (derivedPtr)
        {
            ptr.release();
            return SharedPtr<Derived>(derivedPtr);
        }
        return SharedPtr<Derived>(nullptr);
    }

} // namespace EgLab