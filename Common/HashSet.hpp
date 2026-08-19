#pragma once

#include "Common/HashTable.hpp"

namespace EgLab::Common
{

    template <class T, typename Hash = hash<T>, typename Equal = equal<T>,
              class Allocator = StaticSizeAllocator<T>>
    class HashSet : public HashTable<T, Hash, Equal, Allocator>
    {
        VALUE_ALIAS(T)
    public:
        // using HashSetIterator = Iterator<HashSet<T, Hash, Equal, Allocator>>;
        // using HashSetCIterator = CIterator<HashSet<T, Hash, Equal, Allocator>>;
        // using Element = List<ValueType, StaticSizeAllocator<ListNode<T>>>;
        // using BucketType = DynamicArray<Element, StaticSizeAllocator<Element>>;

        HashSet() : HashTable<T, Hash, Equal, Allocator>()
        {
        }

        bool insert(const ValueType &key)
        {
            if ((this->_size + 1) > this->bucketsSize() * 1.0)
            {
                this->rehash(this->_size + 1);
            }

            size_t index = this->hasher(key) % this->bucketsSize();
            for (const auto &k : this->buckets[index])
            {
                if (this->equaler(k, key)) return false;
            }
            this->buckets[index].pushBack(key);
            ++this->_size;
            return true;
        }

        bool erase(ValueCRef key)
        {
            size_t index = this->hasher(key) % this->bucketsSize();
            auto &bucket = this->buckets[index];
            for (auto it = bucket.begin(); it != bucket.end(); ++it)
            {
                if (equaler(*it, key))
                {
                    bucket.erase(it);
                    --this->_size;
                    return true;
                }
            }
            return false;
        }
    };

} // namespace EgLab::Common