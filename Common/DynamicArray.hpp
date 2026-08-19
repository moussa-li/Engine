#pragma once

#include "Common/Container.hpp"
#include "Common/Exception.hpp"
#include "Common/MacroUtils.hpp"
#include "Common/MemAllocator.hpp"

namespace EgLab::Common
{
    template <class T, class Allocator = StaticSizeAllocator<T>>
    class DynamicArray : public Container<DynamicArray<T, Allocator>>
    {
    public:
        using DynamicArrayIterator = Iterator<DynamicArray<T, Allocator>>;
        using DynamicArrayCIterator = CIterator<DynamicArray<T, Allocator>>;

        VALUE_ALIAS(T)
    public:
        DynamicArray() {};

        inline bool empty() const
        {
            return _size == 0;
        }

        inline size_t size() const
        {
            return _size;
        }

        inline size_t capacity() const
        {
            return _capacity;
        }

        void resize(size_t size)
        {
            if (size > _capacity)
            {
                size_t newCapacity = _capacity ? _capacity : 1;
                while (newCapacity < size)
                {
                    newCapacity *= _incraseFactor;
                }
                ValuePtr newDatas = (ValuePtr)(allocator.alloc(newCapacity));
                for (size_t i = 0; i < _size; i++)
                {
                    new (newDatas + i) ValueType(move(*(_start + i)));
                    //(_start + i)->~ValueType();
                }
                if (_datas != nullptr) allocator.free(_datas);
                _datas = newDatas;
                _capacity = newCapacity;
                _start = _datas;
                _tail = _start + _capacity;
                _end = _start + _size;
            }
            for (int i = _size; i < size; i++)
            {
                new (_datas + i) ValueType();
            }
            _size = size;
            _end = _start + _size;
        }

        void reserve(size_t size)
        {
            if (size > _capacity)
            {
                size_t newCapacity = _capacity;
                while (newCapacity < size)
                {
                    newCapacity *= _incraseFactor;
                }
                ValuePtr newDatas = static_cast<ValuePtr>(allocator.alloc(newCapacity));
                for (size_t i = 0; i < _size; i++)
                {
                    new (newDatas + i) T(move(*(_start + i)));
                    (_start + i)->~T();
                }
                allocator.free(_datas);
                _datas = newDatas;
                _capacity = newCapacity;
                _start = _datas;
                _tail = _start + _capacity;
                _end = _start + _size;
            }
        }

        void pushBack(T &data)
        {
            if (_datas == nullptr)
            {
                _capacity = 2;
                _datas = static_cast<ValuePtr>(allocator.alloc(_capacity));
                _start = _datas;
                _tail = _start + _capacity;
                _end = _start;
            }
            if (_end == _tail)
            {
                size_t newCapacity = _capacity * _incraseFactor;
                ValuePtr newDatas = static_cast<ValuePtr>(allocator.alloc(newCapacity));
                for (size_t i = 0; i < _size; i++)
                {
                    new (newDatas + i) T(move(*(_start + i)));
                    (_start + i)->~T();
                }
                allocator.free(_datas);
                _datas = newDatas;
                _capacity = newCapacity;
                _start = _datas;
                _tail = _start + _capacity;
                _end = _start + _size;
            }

            new (_end) T(data);
            ++_size;
            ++_end;
        }

        void pushBack(T &&data)
        {
            if (_datas == nullptr)
            {
                _capacity = 2;
                _datas = static_cast<ValuePtr>(allocator.alloc(_capacity));
                _start = _datas;
                _tail = _start + _capacity;
                _end = _start;
            }
            if (_end == _tail)
            {
                size_t newCapacity = _capacity * _incraseFactor;
                ValuePtr newDatas = static_cast<ValuePtr>(allocator.alloc(newCapacity));
                for (size_t i = 0; i < _size; i++)
                {
                    new (newDatas + i) T(move(*(_start + i)));
                    (_start + i)->~T();
                }
                allocator.free(_datas);
                _datas = newDatas;
                _capacity = newCapacity;
                _start = _datas;
                _tail = _start + _capacity;
                _end = _start + _size;
            }

            new (_end) T(move(data));
            ++_size;
            ++_end;
        }

        void popBack()
        {
            if (empty()) return;

            _end--;
            _size--;
        }

        DynamicArray(DynamicArray &&other)
        {
            if (this != &other)
            {
                allocator.free(_datas, _capacity);

                _datas = other._datas;
                _start = other._start;
                _end = other._end;
                _tail = other._tail;
                _size = other._size;
                _capacity = other._capacity;
                _incraseFactor = other._incraseFactor;

                other._datas = nullptr;
                other._start = nullptr;
                other._end = nullptr;
                other._tail = nullptr;
                other._size = 0;
                other._capacity = 0;
            }
        }

        DynamicArray(const DynamicArray &other)
        {
            if (this != &other)
            {
                if (capacity() < other.size())
                {
                    ValuePtr newDatas;
                    allocator.free(_datas, _capacity);

                    newDatas = static_cast<ValuePtr>(allocator.alloc(other.size()));
                    _datas = newDatas;
                }

                _capacity = other._size;
                _start = _datas;
                _tail = _start + _capacity;
                _size = other._size;
                _end = _start + _size;
                _incraseFactor = other._incraseFactor;
                for (size_t i = 0; i < _size; i++)
                {
                    new (_datas + i) T(*(other._start + i));
                }
            }
        }

        DynamicArray &operator=(DynamicArray &&other) noexcept
        {
            if (this != &other)
            {
                allocator.free(_datas, _capacity);

                _datas = other._datas;
                _start = other._start;
                _end = other._end;
                _tail = other._tail;
                _size = other._size;
                _capacity = other._capacity;
                _incraseFactor = other._incraseFactor;

                other._datas = nullptr;
                other._start = nullptr;
                other._end = nullptr;
                other._tail = nullptr;
                other._size = 0;
                other._capacity = 0;
            }
            return *this;
        }

        DynamicArray &operator=(const DynamicArray &other)
        {
            if (this != &other)
            {
                if (capacity() < other.size())
                {
                    ValuePtr newDatas;
                    allocator.free(_datas, _capacity);

                    newDatas = static_cast<ValuePtr>(allocator.alloc(other.size()));
                    _datas = newDatas;
                }

                _capacity = other._size;
                _start = _datas;
                _size = other._size;
                _tail = _start + _capacity;
                _end = _start + _size;
                _incraseFactor = other._incraseFactor;
                for (size_t i = 0; i < _size; i++)
                {
                    new (_datas + i) T(*(other._start + i));
                }
            }
            return *this;
        }

        T &operator[](size_t index)
        {
            return *(_start + index);
        }

        const T &operator[](size_t index) const
        {
            return *(_start + index);
        }

        ValueRef front()
        {
            return *_start;
        }

        ValueCRef front() const
        {
            return *_start;
        }

        ValueRef back()
        {
            return *(_start + _size - 1);
        }

        ValueCRef back() const
        {
            return *(_start + _size - 1);
        }

        Iterator<DynamicArray<T, Allocator>> begin() override
        {
            return move(DynamicArrayIterator(*this));
        }

        Iterator<DynamicArray<T, Allocator>> end() override
        {
            DynamicArrayIterator it(*this);
            it.current = _end;
            return move(it);
        }

        CIterator<DynamicArray<T, Allocator>> begin() const override
        {
            return move(DynamicArrayCIterator(*this));
        }

        CIterator<DynamicArray<T, Allocator>> end() const override
        {
            DynamicArrayCIterator it(*this);
            it.current = _end;
            return move(it);
        }

        ValuePtr data() const
        {
            return _datas;
        }

    private:
        ValuePtr _datas{nullptr};
        ValuePtr _start{nullptr};
        ValuePtr _end{nullptr};
        ValuePtr _tail{nullptr};
        size_t _size{0};
        size_t _capacity{0};

        size_t _incraseFactor{2};

        friend class Iterator<DynamicArray<T, Allocator>>;
        friend class CIterator<DynamicArray<T, Allocator>>;
        static Allocator allocator;
    };

    template <class T, class Allocator>
    class Iterator<DynamicArray<T, Allocator>>
    {
        VALUE_ALIAS(T)
    public:
        bool hasNext() const
        {
            return current < array._end;
        }

        ValueRef next()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("DynamicArray iterator reached the end");
            }
            return *(current++);
        }

        ValueCRef operator*() const
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("DynamicArray iterator reached the end");
            }
            return *current;
        }

        ValueRef operator*()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No current element in DynamicArray iterator");
            }
            return *current;
        }

        ValueRef data()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No current element in DynamicArray iterator");
            }
            return *current;
        }

        Iterator<DynamicArray<T, Allocator>>(DynamicArray<T, Allocator> &arr)
            : array(arr), current(arr._start)
        {
        }

        Iterator<DynamicArray<T, Allocator>>(const Iterator &other)
            : array(other.array), current(other.current)
        {
        }

        Iterator<DynamicArray<T, Allocator>> operator++()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("DynamicArray iterator reached the end");
            }
            current++;

            return *this;
        }

        bool operator==(const Iterator<DynamicArray<T, Allocator>> &other) const
        {
            return current == other.current;
        }

    private:
        friend class DynamicArray<T, Allocator>;
        ValuePtr current;
        DynamicArray<T, Allocator> &array;
    };

    template <class T, class Allocator>
    class CIterator<DynamicArray<T, Allocator>>
    {
        VALUE_ALIAS(T)
    public:
        bool hasNext() const
        {
            return current < array._end;
        }

        ValueCRef next()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("DynamicArray iterator reached the end");
            }
            return *(current++);
        }

        ValueCRef operator*() const
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("DynamicArray iterator reached the end");
            }
            return *current;
        }

        ValueCRef operator*()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No current element in DynamicArray iterator");
            }
            return *current;
        }

        ValueCRef data()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No current element in DynamicArray iterator");
            }
            return *current;
        }

        CIterator<DynamicArray<T, Allocator>>(const DynamicArray<T, Allocator> &arr)
            : array(arr), current(arr._start)
        {
        }

        CIterator<DynamicArray<T, Allocator>>(const CIterator &other)
            : array(other.array), current(other.current)
        {
        }

        CIterator<DynamicArray<T, Allocator>> operator++()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("DynamicArray iterator reached the end");
            }
            current++;

            return *this;
        }

        bool operator==(const CIterator<DynamicArray<T, Allocator>> &other) const
        {
            return current == other.current;
        }

        size_t index() const
        {
            if (hasNext() == false) return -1;
            return current - array._start;
        }

    private:
        friend class DynamicArray<T, Allocator>;
        ValuePtr current;
        const DynamicArray<T, Allocator> &array;
    };

    template <class T, class Allocator>
    Allocator DynamicArray<T, Allocator>::allocator;
} // namespace EgLab::Common