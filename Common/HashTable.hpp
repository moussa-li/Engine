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
    class HashTable : public Container<HashTable<T>>
    {
        VALUE_ALIAS(T)
    public:
        HashTable()
        {
        }

        bool insert(const ValueType &key)
        {
            if ((_size + 1) > bucketsSize() * 1.0)
            {
                rehash();
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

        Iterator<HashTable<T, Hash, Allocator>> *begin() override
        {
            return new HashTableIterator(*this);
        }

        Iterator<HashTable<T, Hash, Allocator>> *end() override
        {
            HashTableIterator *it = new HashTableIterator(*this);
            it->_bucketIndex = bucketsSize();
            return it;
        }

        class HashTableIterator : public Iterator<HashTable<T, Hash, Allocator>>
        {
        public:
            bool hasNext() const override
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
            ValueRef next() override
            {
                if (!hasNext())
                {
                    throw OutOfMemoryException("No more elements in HashTable iterator");
                }
                ValueRef value = *_bucketIterator;
                ++_bucketIterator;
                return value;
            }

            ValueCRef operator*() const override
            {
                if (!hasNext())
                {
                    throw OutOfMemoryException("No current element in HashTable iterator");
                }
                return *_bucketIterator;
            }

            ValueRef operator*() override
            {
                if (!hasNext())
                {
                    throw OutOfMemoryException("No current element in HashTable iterator");
                }
                return *_bucketIterator;
            }

            HashTableIterator(const HashTable<T, Hash, Allocator> &ht) : _table(ht), _bucketIndex(0)
            {
                if (_table.bucketsSize() > 0)
                {
                    _bucketIterator = _table.buckets[0].begin();
                }
            }

        private:
            const HashTable<T, Hash, Allocator> &_table;
            size_t _bucketIndex;
            typename DynamicArray<List<ValueType>>::ValueRef::Iterator _bucketIterator;
        };

    private:
        DynamicArray<List<ValueType>> buckets;
        size_t _size;
        Hash hasher;

        void rehash()
        {
        }
    };

} // namespace EgLab