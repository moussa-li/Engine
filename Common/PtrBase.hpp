#pragma once

namespace EgLab
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
} // namespace EgLab