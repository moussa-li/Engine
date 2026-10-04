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

namespace EgLab::Common
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
        virtual ~IntrusiveRef() = default;

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
            releaseRef();
        }

        SharedPtr(SharedPtr &&ptr) noexcept : PtrBase<T>(ptr._ptr), _ref(ptr._ref)
        {
            ptr._ptr = nullptr;
            ptr._ref = nullptr;
        }

        // explicit SharedPtr(T *ptr)
        // {
        //     this->_ptr = ptr;
        //     if (this->_ptr)
        //     {
        //         ptr->addRef();
        //     }
        // }

        SharedPtr(const SharedPtr &other) : PtrBase<T>(other._ptr), _ref(other._ref)
        {
            if (_ref)
            {
                _ref->addRef();
            }
        }

        template <typename Derived>
        SharedPtr(const SharedPtr<Derived> &other) : PtrBase<T>(other._ptr), _ref(other._ref)
        {
            if (_ref)
            {
                _ref->addRef();
            }
        }

        template <typename U, typename... Args,
                  typename = enableIf_t<!isSame<decay_t<U>, SharedPtr<T>>::value &&
                                        !isSame<decay_t<U>, std::nullptr_t>::value>>
        explicit SharedPtr(U &&firstArg, Args &&...args)
            : PtrBase<T>(static_cast<T *>(new TRef(forward<U>(firstArg), forward<Args>(args)...))),
              _ref(static_cast<TRef *>(this->_ptr))
        {
            _ref->addRef();
        }

        SharedPtr(T *ptr) : PtrBase<T>(ptr), _ref(ptr ? static_cast<TRef *>(ptr) : nullptr)
        {
            if (_ref)
            {
                _ref->addRef();
            }
        }

        void operator=(T *ptr)
        {
            assign(ptr, ptr ? static_cast<TRef *>(ptr) : nullptr);
        }

        SharedPtr<T> &operator=(const SharedPtr<T> &other)
        {
            assign(other._ptr, other._ref);
            return *this;
        }

        SharedPtr<T> &operator=(SharedPtr<T> &&other) noexcept
        {
            if (this != &other)
            {
                releaseRef();
                this->_ptr = other._ptr;
                _ref = other._ref;
                other._ptr = nullptr;
                other._ref = nullptr;
            }
            return *this;
        }

        bool operator==(const SharedPtr<T> &other) const noexcept
        {
            return this->_ptr == other._ptr;
        }

        bool operator!=(const SharedPtr<T> &other) const noexcept
        {
            return this->_ptr != other._ptr;
        }

        explicit operator bool() const noexcept
        {
            return this->_ptr != nullptr;
        }

    private:
        void assign(T *ptr, IntrusiveRef *ref)
        {
            if (ref)
            {
                ref->addRef();
            }
            releaseRef();
            this->_ptr = ptr;
            _ref = ref;
        }

        void releaseRef()
        {
            if (_ref)
            {
                _ref->subRef();
                if (_ref->getRef() == 0)
                {
                    delete _ref;
                }
                _ref = nullptr;
            }
        }

        class TRef : public T, virtual public IntrusiveRef
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

        };

        IntrusiveRef *_ref{nullptr};

        template <typename Derived>
        friend class SharedPtr;

        template <class Derived, typename Base>
        friend SharedPtr<Derived> dynamicSharedPtrCast(SharedPtr<Base> &ptr);

        template <class U, class... Args>
        friend SharedPtr<U> makeShared(Args &&...args);

        template <typename Derived>
        SharedPtr(Derived *ptr, IntrusiveRef *ref) : PtrBase<T>(ptr), _ref(ref)
        {
            if (_ref)
            {
                _ref->addRef();
            }
        }
    };

    template <class T, class... Args>
    SharedPtr<T> makeShared(Args &&...args)
    {
        return SharedPtr<T>((T *)new typename SharedPtr<T>::TRef(forward<Args>(args)...));
    }

    template <class Derived, typename Base>
    SharedPtr<Derived> dynamicSharedPtrCast(SharedPtr<Base> &ptr)
    {
        Derived *derivedPtr = dynamic_cast<Derived *>(ptr.get());
        if (derivedPtr)
        {
            return SharedPtr<Derived>(derivedPtr, ptr._ref);
        }
        return SharedPtr<Derived>(nullptr);
    }

} // namespace EgLab::Common