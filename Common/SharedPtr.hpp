#pragma once
/**
 * @file SharedPtr.hpp
 * @author Moussa-Li
 * @brief The usage is the same as shared ptr,
 *  but it is an embedded smart pointer,
 *  so you need to let T drived from Object
 * @date 2026-01-18
 */

#include <cstddef>

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

    class IntrusiveRef
    {
    public:
        inline size_t getRef()
        {
            return ref;
        }

    protected:
        ~IntrusiveRef() {};

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
                TRef *ptr = static_cast<TRef *>(this->_ptr);
                ptr->subRef();
                if (ptr->getRef() == 0)
                {
                    delete this->_ptr;
                    this->_ptr = nullptr;
                }
            }
        }

        SharedPtr(SharedPtr &&ptr) noexcept
        {
            this->operator=(ptr.get());
            // return *this;
        }

        // explicit SharedPtr(T *ptr)
        // {
        //     this->_ptr = ptr;
        //     if (this->_ptr)
        //     {
        //         ptr->addRef();
        //     }
        // }

        SharedPtr(const SharedPtr &other)
        {
            // TRef *otherPtr = static_cast<TRef *>(other->_ptr);
            this->_ptr = other._ptr;
            TRef *ptr = static_cast<TRef *>(this->_ptr);
            if (ptr)
            {
                ptr->addRef();
            }
        }

        template <typename Derived>
        SharedPtr(const SharedPtr<Derived> &other)
        {
            this->_ptr = other._ptr;
            auto *ptr = static_cast<SharedPtr<Derived>::TRef *>(this->_ptr);
            if (ptr)
            {
                ptr->addRef();
            }
        }

        template <typename U, typename... Args,
                  typename = enableIf_t<!isSame<decay_t<U>, SharedPtr<T>>::value &&
                                        !isSame<decay_t<U>, std::nullptr_t>::value>>
        explicit SharedPtr(U &&firstArg, Args &&...args)
            : PtrBase<T>(static_cast<T *>(new TRef(forward<U>(firstArg), forward<Args>(args)...)))
        {
            TRef *ptr = static_cast<TRef *>(this->_ptr);
            ptr->addRef();
        }

        SharedPtr(T *ptr) : PtrBase<T>(ptr)
        {
            TRef *tptr = static_cast<TRef *>(this->_ptr);
            if (tptr == nullptr) return;
            tptr->addRef();
        }

        void operator=(T *ptr)
        {
            TRef *newPtr = static_cast<TRef *>(ptr);
            TRef *lastPtr = static_cast<TRef *>(this->_ptr);
            this->_ptr = ptr;
            if (this->_ptr)
            {
                newPtr->addRef();
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
        class TRef : public T, public IntrusiveRef
        {
        public:
            template <typename... Args>
            TRef(Args &&...args) : T(forward<Args>(args)...)
            {
            }

            TRef(const T &other) : T(other)
            {
            }

            TRef(T &&other) : T(other)
            {
            }

            ~TRef()
            {
            }
        };

        template <typename Derived>
        friend class SharedPtr;

        template <class U, class... Args>
        friend SharedPtr<U> makeShared(Args &&...args);
    };

    template <class T, class... Args>
    SharedPtr<T> makeShared(Args &&...args)
    {
        return SharedPtr<T>((T *)new typename SharedPtr<T>::TRef(forward<Args>(args)...));
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