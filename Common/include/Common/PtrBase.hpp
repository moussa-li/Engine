#pragma once

#include <cstddef>

namespace EgLab::Common
{
    template <class T>
    class PtrBase
    {
    public:
        PtrBase(T* ptr = nullptr) : _ptr(ptr)
        {
        }

        T* get() const
        {
            return _ptr;
        }

        T* release()
        {
            T* temp = _ptr;
            _ptr = nullptr;
            return temp;
        }

        bool operator==(std::nullptr_t) const
        {
            return this->_ptr == nullptr;
        }

        explicit operator bool() const
        {
            return this->_ptr != nullptr;
        }

        T& operator*() const
        {
            return *_ptr;
        }

        T* operator->() const
        {
            return _ptr;
        }

    protected:
        T* _ptr;
    };
} // namespace EgLab::Common