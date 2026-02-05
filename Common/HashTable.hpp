#pragma once

#include "Common/Container.hpp"
#include "Common/DynamicArray.hpp"
#include "Common/Exception.hpp"
#include "Common/Iterator.hpp"
#include "Common/List.hpp"
#include "Common/MemAllocator.hpp"
#include "Common/String.hpp"

namespace EgLab
{

    template <typename T>
    struct hash;

    template <>
    struct hash<int>
    {
        size_t operator()(int x) const
        {
            return static_cast<size_t>(x);
        }
    };

    class String;
    template <>
    struct hash<String>
    {
        size_t operator()(const String &s) const
        {
            size_t hash = 14695981039346656037ULL; // FNV offset basis
            for (char c : s)
            {
                hash ^= static_cast<size_t>(c);
                hash *= 1099511628211ULL; // FNV prime
            }
            return hash;
        }
    };

    template <class T, typename Hash = hash<T>, typename Equal = equal<T>,
              class Allocator = StaticSizeAllocator<T>>
    class HashTable : public Container<HashTable<T, Hash, Equal, Allocator>>
    {
        VALUE_ALIAS(T)
    public:
        using HashTableIterator = Iterator<HashTable<T, Hash, Equal, Allocator>>;
        using HashTableCIterator = CIterator<HashTable<T, Hash, Equal, Allocator>>;
        using Element = List<ValueType, StaticSizeAllocator<ListNode<T>>>;
        using BucketType = DynamicArray<Element, StaticSizeAllocator<Element>>;

        inline size_t bucketsSize() const
        {
            return buckets.size();
        }

        inline size_t size() const
        {
            return _size;
        }

        inline bool empty() const
        {
            return _size == 0;
        }

        Iterator<HashTable<T, Hash, Equal, Allocator>> begin() override
        {
            return move(HashTableIterator(*this));
        }

        Iterator<HashTable<T, Hash, Equal, Allocator>> end() override
        {
            HashTableIterator it(*this);
            it._bucketIndex = this->bucketsSize();
            if (it._bucketIndex != 0)
            {
                it._bucketIterator =
                    this->buckets[this->bucketsSize() - 1].end(); // TODO: change to back()
            }
            return (move(it));
        }

        CIterator<HashTable<T, Hash, Equal, Allocator>> begin() const override
        {
            return move(HashTableCIterator(*this));
        }

        CIterator<HashTable<T, Hash, Equal, Allocator>> end() const override
        {
            HashTableCIterator it(*this);
            it._bucketIndex = this->bucketsSize();
            if (it._bucketIndex != 0)
            {
                it._bucketIterator =
                    this->buckets[this->bucketsSize() - 1].end(); // TODO: change to back()
            }
            return (move(it));
        }

        HashTable() : _size(0), buckets()
        {
        }

    protected:
        BucketType buckets;
        size_t _size;
        Hash hasher;
        Equal equaler;
        friend class Iterator<HashTable<T, Hash, Equal, Allocator>>;
        friend class CIterator<HashTable<T, Hash, Equal, Allocator>>;

        void rehash(size_t n)
        {
            size_t newBucketSize = calcBucketSize(n);
            if (newBucketSize <= buckets.size()) return;

            size_t bucketSize = buckets.size();
            buckets.resize(newBucketSize);

            typename Element::IteratorT its[newBucketSize];
            for (size_t i = 0; i < bucketSize; ++i)
            {
                its[i] = buckets[i].begin();
            }
            for (size_t i = bucketSize; i < newBucketSize; ++i)
            {
                its[i] = buckets[i].end();
            }

            for (size_t i = 0; i < bucketSize; ++i)
            {
                auto &it = its[i];
                Element &e = buckets[i];
                while (it != e.end())
                {
                    size_t newIdx = hasher(*it) % newBucketSize;
                    if (newIdx == i)
                    {
                        ++it;
                        continue;
                    }
                    auto &newIt = its[newIdx];
                    auto &bucket = buckets[newIdx];
                    if (newIt == bucket.end())
                    {
                        auto *n = buckets[i].pop(it);
                        buckets[newIdx].pushBack(n);
                    }
                    else
                    {
                        swap(*it, *newIt);
                        ++it;
                    }
                }
            }
        }

        void setItData(
            Iterator<HashTable<T, Hash, Equal, Allocator>> &it, const size_t &bucketIndex,
            const List<ValueType, StaticSizeAllocator<ListNode<T>>>::ListCIterator &bucketIterator)
        {
            it._bucketIndex = bucketIndex;
            it._bucketIterator = bucketIterator;
        }

        void setItData(
            CIterator<HashTable<T, Hash, Equal, Allocator>> &it, const size_t &bucketIndex,
            const List<ValueType, StaticSizeAllocator<ListNode<T>>>::ListCIterator &bucketIterator)
        {
            it._bucketIndex = bucketIndex;
            it._bucketIterator = bucketIterator;
        }

        size_t calcBucketSize(size_t n)
        {
            if (n == 0) return 1;
            --n;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;
#if SIZE_MAX > UINT32_MAX
            n |= n >> 32; // 64位系统
#endif
            return n + 1;
        }
    };

    template <class T, typename Hash, class Equal, class Allocator>
    class Iterator<HashTable<T, Hash, Equal, Allocator>>
    {
        VALUE_ALIAS(T)
    public:
        bool hasNext() const
        {
            size_t bucketIndex = _bucketIndex;
            auto bucketIt = _table.buckets[bucketIndex].begin();
            while (bucketIndex < _table.bucketsSize())
            {
                if (bucketIt != _table.buckets[bucketIndex].end())
                {
                    return true;
                }
                ++bucketIndex;
                if (bucketIndex < _table.bucketsSize())
                {
                    bucketIt = _table.buckets[bucketIndex].begin();
                }
            }
            return false;
        }
        ValueRef next()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No more elements in HashTable iterator");
            }
            ValueRef value = *_bucketIterator;
            ++*this;
            return value;
        }

        ValueCRef operator*() const
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No current element in HashTable iterator");
            }
            return *_bucketIterator;
        }

        ValueCRef operator*()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No current element in HashTable iterator");
            }
            return *_bucketIterator;
        }

        bool operator==(const Iterator<HashTable<T, Hash, Equal, Allocator>> &other) const
        {
            if (_bucketIndex != other._bucketIndex) return false;
            return _bucketIterator == other._bucketIterator;
        }

        Iterator<HashTable<T, Hash, Equal, Allocator>> operator++()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException();
            }
            ++_bucketIterator;
            while (_bucketIterator == _table.buckets[_bucketIndex].end() &&
                   ++_bucketIndex != _table.bucketsSize())
            {
                _bucketIterator = _table.buckets[_bucketIndex].begin();
            }
            return *this;
        }

        ValueRef data()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No current element in HashTable iterator");
            }
            return *_bucketIterator;
        }

        Iterator<HashTable<T, Hash, Equal, Allocator>>(
            Iterator<HashTable<T, Hash, Equal, Allocator>> &&other)
            : _table(other._table),
              _bucketIndex(other._bucketIndex),
              _bucketIterator(other._bucketIterator)
        {
        }

        Iterator<HashTable<T, Hash, Equal, Allocator>>(
            const Iterator<HashTable<T, Hash, Equal, Allocator>> &other)
            : _table(other._table),
              _bucketIndex(other._bucketIndex),
              _bucketIterator(other._bucketIterator)
        {
        }

        Iterator<HashTable<T, Hash, Equal, Allocator>>(
            CIterator<HashTable<T, Hash, Equal, Allocator>> &&other)
            : _table(other._table),
              _bucketIndex(other._bucketIndex),
              _bucketIterator(other._bucketIterator)
        {
        }

        Iterator<HashTable<T, Hash, Equal, Allocator>> operator=(
            const Iterator<HashTable<T, Hash, Equal, Allocator>> &other)
        {
            if (&_table != &other._table)
            {
                LOG(ERROR) << "error iterator for hashTable, becaus the error table";
                return *this;
            }
            _bucketIndex = other._bucketIndex;
            _bucketIterator = other._bucketIterator;
            return *this;
        }

        Iterator<HashTable<T, Hash, Equal, Allocator>>(
            const CIterator<HashTable<T, Hash, Equal, Allocator>> &other)
            : _table(other._table),
              _bucketIndex(other._bucketIndex),
              _bucketIterator(other._bucketIterator)
        {
        }

        Iterator<HashTable<T, Hash, Equal, Allocator>>(
            const HashTable<T, Hash, Equal, Allocator> &ht)
            : _table(ht), _bucketIndex(0), _bucketIterator()
        {
            for (auto it = _table.buckets.begin(); it != _table.buckets.end(); ++it)
            {
                if ((*it).empty() == false)
                {
                    _bucketIterator = (*it).begin();
                    _bucketIndex = it.index();
                    break;
                }
            }
        }

    private:
        const HashTable<T, Hash, Equal, Allocator> &_table;
        size_t _bucketIndex;
        typename List<ValueType, StaticSizeAllocator<ListNode<T>>>::ListCIterator _bucketIterator;
        friend class HashTable<T, Hash, Equal, Allocator>;
    };

    template <class T, typename Hash, class Equal, class Allocator>
    class CIterator<HashTable<T, Hash, Equal, Allocator>>
    {
        VALUE_ALIAS(T)
    public:
        bool hasNext() const
        {
            size_t bucketIndex = _bucketIndex;
            while (bucketIndex < _table.bucketsSize())
            {
                if (_bucketIterator != _table.buckets[bucketIndex].end())
                {
                    return true;
                }
                ++bucketIndex;
                if (bucketIndex < _table.bucketsSize())
                {
                    _bucketIterator = _table.buckets[bucketIndex].begin();
                }
            }
            return false;
        }
        ValueCRef next()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No more elements in HashTable iterator");
            }
            ValueCRef value = *_bucketIterator;
            ++_bucketIterator;
            return value;
        }

        ValueCRef operator*() const
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No current element in HashTable iterator");
            }
            return *_bucketIterator;
        }

        ValueCRef operator*()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No current element in HashTable iterator");
            }
            return *_bucketIterator;
        }

        bool operator==(const CIterator<HashTable<T, Hash, Equal, Allocator>> &other) const
        {
            if (_bucketIndex != other._bucketIndex) return false;
            return _bucketIterator == other._bucketIterator;
        }

        ValueCRef data()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No current element in HashTable iterator");
            }
            return *_bucketIterator;
        }

        CIterator<HashTable<T, Hash, Equal, Allocator>>(
            const CIterator<HashTable<T, Hash, Equal, Allocator>> &other)
            : _table(other._table),
              _bucketIndex(other._bucketIndex),
              _bucketIterator(other._bucketIterator)
        {
        }

        CIterator<HashTable<T, Hash, Equal, Allocator>> operator=(
            const CIterator<HashTable<T, Hash, Equal, Allocator>> &other)
        {
            _table = other._table;
            _bucketIndex = other._bucketIndex;
            _bucketIterator = other._bucketIterator;
            return *this;
        }

        CIterator<HashTable<T, Hash, Equal, Allocator>>(
            const HashTable<T, Hash, Equal, Allocator> &ht)
            : _table(ht), _bucketIndex(0), _bucketIterator()
        {
            for (auto &it : _table.buckets)
            {
                if (it.empty() == false)
                {
                    _bucketIterator = it.begin();
                }
            }
        }

    private:
        const HashTable<T, Hash, Equal, Allocator> &_table;
        size_t _bucketIndex;
        typename List<ValueType, StaticSizeAllocator<ListNode<T>>>::ListCIterator _bucketIterator;
        friend class HashTable<T, Hash, Equal, Allocator>;
    };

} // namespace EgLab