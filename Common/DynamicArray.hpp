#pragma once

#include "Common/Container.hpp"
#include "Common/Exception.hpp"
#include "Common/MacroUtils.hpp"
#include "Common/MemAllocator.hpp"

namespace EgLab
{
    template <class T, class Allocator = StaticSizeAllocator<T>>
    class DynamicArray : public Container<DynamicArray<T>>
    {
    public:
        using ValuePtr = T *;
        using ValueCPtr = const T *;
        using ValueCPtrC = const T *const;
        using ValueRef = T &;
        using ValueCRef = const T &;
        using ValueType = T;

        // VALUE_ALIAS(T)
    public:
        DynamicArray() : _size(0), _capacity(0) {};

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
                ValuePtr newDatas = static_cast<ValuePtr>(allocator.alloc(newCapacity));
                for (size_t i = 0; i < _size; i++)
                {
                    new (newDatas + i) T(std::move(*(_start + i)));
                    (_start + i)->~T();
                }
                if (_datas != nullptr) allocator.free(_datas);
                _datas = newDatas;
                _capacity = newCapacity;
                _start = _datas;
                _tail = _start + _capacity;
                _end = _start + _size;
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
                    new (newDatas + i) T(std::move(*(_start + i)));
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
                    new (newDatas + i) T(std::move(*(_start + i)));
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
                    new (newDatas + i) T(std::move(*(_start + i)));
                    (_start + i)->~T();
                }
                allocator.free(_datas);
                _datas = newDatas;
                _capacity = newCapacity;
                _start = _datas;
                _tail = _start + _capacity;
                _end = _start + _size;
            }

            new (_end) T(std::move(data));
            ++_size;
            ++_end;
        }

        T &operator[](size_t index)
        {
            return *(_start + index);
        }

        const T &operator[](size_t index) const
        {
            return *(_start + index);
        }

        Iterator<DynamicArray<T>> *begin() override
        {
            return new DynamicArrayIterator(*this);
        }

        Iterator<DynamicArray<T>> *end() override
        {
            DynamicArrayIterator *it = new DynamicArrayIterator(*this);
            it->current = _end;
            return it;
        }

        class DynamicArrayIterator : public Iterator<DynamicArray<T>>
        {
        public:
            bool hasNext() const override
            {
                return current < array._end;
            }

            ValueRef next() override
            {
                if (!hasNext())
                {
                    throw OutOfMemoryException("DynamicArray iterator reached the end");
                }
                return *(current++);
            }

            ValueCRef operator*() const override
            {
                if (!hasNext())
                {
                    throw OutOfMemoryException("DynamicArray iterator reached the end");
                }
                return *current;
            }

            ValueRef operator*() override
            {
                if (!hasNext())
                {
                    throw OutOfMemoryException("No current element in DynamicArray iterator");
                }
                return *current;
            }

            DynamicArrayIterator(DynamicArray<T> &arr) : array(arr), current(arr._start)
            {
            }

            DynamicArrayIterator(const DynamicArrayIterator &other)
                : array(other.array), current(other.current)
            {
            }

        private:
            friend class DynamicArray<T>;
            ValuePtr current;
            DynamicArray<T> &array;
        };

    private:
        ValuePtr _datas{nullptr};
        ValuePtr _start{nullptr};
        ValuePtr _end{nullptr};
        ValuePtr _tail{nullptr};
        size_t _size;
        size_t _capacity;

        size_t _incraseFactor{2};

        static Allocator allocator;
    };

    template <class T, class Allocator>
    Allocator DynamicArray<T, Allocator>::allocator;
} // namespace EgLab