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

    template <class T, typename Hash = hash<T>, class Allocator = StaticSizeAllocator<T>>
    class HashSet : public Container<HashSet<T>>
    {
        VALUE_ALIAS(T)
    public:
        using HashSetIterator = Iterator<HashSet<T, Hash, Allocator>>;
        using HashSetCIterator = CIterator<HashSet<T, Hash, Allocator>>;
        using Element = List<ValueType, StaticSizeAllocator<ListNode<T>>>;
        using BucketType = DynamicArray<Element, StaticSizeAllocator<Element>>;

        HashSet() : _size(0), buckets()
        {
        }

        bool insert(const ValueType &key)
        {
            if ((_size + 1) > bucketsSize() * 1.0)
            {
                rehash(_size + 1);
            }

            size_t index = hasher(key) % bucketsSize();
            for (const auto &k : buckets[index])
            {
                if (k == key) return false;
            }
            buckets[index].pushBack(key);
            ++_size;
            return true;
        }

        inline size_t size() const
        {
            return _size;
        }

        inline bool empty() const
        {
            return _size == 0;
        }

        bool erase(ValueCRef key)
        {
            size_t index = hasher(key) % bucketsSize();
            auto &bucket = buckets[index];
            for (auto it = bucket.begin(); it != bucket.end(); ++it)
            {
                if (*it == key)
                {
                    bucket.erase(it);
                    --_size;
                    return true;
                }
            }
            return false;
        }

        inline size_t bucketsSize() const
        {
            return buckets.size();
        }

        Iterator<HashSet<T, Hash, Allocator>> begin() override
        {
            return move(HashSetIterator(*this));
        }
        Iterator<HashSet<T, Hash, Allocator>> end() override
        {
            HashSetIterator it(*this);
            it._bucketIndex = bucketsSize();
            it._bucketIterator = buckets[bucketsSize() - 1].end(); // TODO: change to back()
            return (move(it));
        }

        CIterator<HashSet<T, Hash, Allocator>> begin() const override
        {
            return move(HashSetCIterator(*this));
        }

        CIterator<HashSet<T, Hash, Allocator>> end() const override
        {
            HashSetCIterator it(*this);
            it._bucketIndex = bucketsSize();
            it._bucketIterator = buckets[bucketsSize() - 1].end(); // TODO: change to back()
            return (move(it));
        }

    private:
        BucketType buckets;
        size_t _size;
        Hash hasher;
        friend class Iterator<HashSet<T, Hash, Allocator>>;
        friend class CIterator<HashSet<T, Hash, Allocator>>;

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

    template <class T, typename Hash, class Allocator>
    class Iterator<HashSet<T, Hash, Allocator>>
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
                throw OutOfMemoryException("No more elements in HashSet iterator");
            }
            ValueRef value = *_bucketIterator;
            ++*this;
            return value;
        }

        ValueCRef operator*() const
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No current element in HashSet iterator");
            }
            return *_bucketIterator;
        }

        ValueCRef operator*()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No current element in HashSet iterator");
            }
            return *_bucketIterator;
        }

        bool operator==(const Iterator<HashSet<T, Hash, Allocator>> &other) const
        {
            if (_bucketIndex != other._bucketIndex) return false;
            return _bucketIterator == other._bucketIterator;
        }

        Iterator<HashSet<T, Hash, Allocator>> operator++()
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
                throw OutOfMemoryException("No current element in HashSet iterator");
            }
            return *_bucketIterator;
        }

        Iterator<HashSet<T, Hash, Allocator>>(const Iterator<HashSet<T, Hash, Allocator>> &other)
            : _table(other._table),
              _bucketIndex(other._bucketIndex),
              _bucketIterator(other._bucketIterator)
        {
            // if (_table.bucketsSize() > 0)
            // {
            //     _bucketIterator = _table.buckets[0].begin();
            // }
        }

        Iterator<HashSet<T, Hash, Allocator>>(const HashSet<T, Hash, Allocator> &ht)
            : _table(ht), _bucketIndex(0), _bucketIterator()
        {
            // for (const auto &it : _table.buckets)
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
        const HashSet<T, Hash, Allocator> &_table;
        size_t _bucketIndex;
        typename List<ValueType, StaticSizeAllocator<ListNode<T>>>::ListCIterator _bucketIterator;
        friend class HashSet<T, Hash, Allocator>;
    };

    template <class T, typename Hash, class Allocator>
    class CIterator<HashSet<T, Hash, Allocator>>
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
                throw OutOfMemoryException("No more elements in HashSet iterator");
            }
            ValueCRef value = *_bucketIterator;
            ++_bucketIterator;
            return value;
        }

        ValueCRef operator*() const
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No current element in HashSet iterator");
            }
            return *_bucketIterator;
        }

        ValueCRef operator*()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No current element in HashSet iterator");
            }
            return *_bucketIterator;
        }

        ValueCRef data()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No current element in HashSet iterator");
            }
            return *_bucketIterator;
        }

        CIterator<HashSet<T, Hash, Allocator>>(const CIterator<HashSet<T, Hash, Allocator>> &other)
            : _table(other._table),
              _bucketIndex(other._bucketIndex),
              _bucketIterator(other._bucketIterator)
        {
        }

        CIterator<HashSet<T, Hash, Allocator>>(const HashSet<T, Hash, Allocator> &ht)
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
        const HashSet<T, Hash, Allocator> &_table;
        size_t _bucketIndex;
        typename List<ValueType, StaticSizeAllocator<ListNode<T>>>::ListCIterator _bucketIterator;
        friend class HashSet<T, Hash, Allocator>;
    };

} // namespace EgLab