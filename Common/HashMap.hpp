#pragma once

#include "HashTable.hpp"
#include "Pair.hpp"

// std::unordered_map<int, int> m;
namespace EgLab
{
    template <class F, class S>
    struct pairHash : public hash<F>
    {
        size_t operator()(Pair<F, S> data) const
        {
            return hash<F>::operator()(data.first);
        }

        size_t operator()(F key) const
        {
            return hash<F>::operator()(key);
        }
    };

    template <class F, class S>
    struct pairEqual
    {
        constexpr bool operator()(const Pair<F, S> &left, const Pair<F, S> &right) const
        {
            return left.first == right.first;
        }

        constexpr bool operator()(const Pair<F, S> &left, const F &right) const
        {
            return left.first == right;
        }

        constexpr bool operator()(const F &left, const Pair<F, S> &right) const
        {
            return left == right.first;
        }

        constexpr bool operator()(const F &left, const F &right) const
        {
            return left == right;
        }
    };

    template <typename Key, typename Value, typename Hash = pairHash<Key, Value>,
              typename Equal = pairEqual<Key, Value>,
              typename Allocator = StaticSizeAllocator<Pair<Key, Value>>>
    class HashMap : public HashTable<Pair<Key, Value>, Hash, Equal, Allocator>
    {
        using DataType = Pair<Key, Value>;
        VALUE_ALIAS(DataType)

    public:
        HashMap() : HashTable<Pair<Key, Value>, Hash, Equal, Allocator>()
        {
        }

        Value &operator[](const Key &key)
        {
            size_t index = this->hasher(key) % this->bucketsSize();
            auto &bucket = this->buckets[index];

            for (auto it = bucket.begin(); it != bucket.end(); ++it)
            {
                if (this->equaler(*it, key))
                {
                    --this->_size;
                    return (*it).second;
                }
            }

            insert({key, Value()});
            return (*this)[key];
        }

        Value &operator[](Key &&key)
        {
            size_t index = this->hasher(key) % this->bucketsSize();
            auto &bucket = this->buckets[index];

            for (auto it = bucket.begin(); it != bucket.end(); ++it)
            {
                if (this->equaler(*it, key))
                {
                    --this->_size;
                    return (*it).second;
                }
            }

            insert({key, Value()});
            return (*this)[key];
        }

        bool insert(const DataType &data)
        {
            if ((this->_size + 1) > this->bucketsSize() * 1.0)
            {
                this->rehash(this->_size + 1);
            }

            size_t index = this->hasher(data) % this->bucketsSize();
            for (const auto &k : this->buckets[index])
            {
                if (this->equaler(k, data)) return false;
            }
            this->buckets[index].pushBack(data);
            ++this->_size;
            return true;
        }

        bool erase(const Key &key)
        {
            size_t index = this->hasher(key) % this->bucketsSize();
            auto &bucket = this->buckets[index];
            for (auto it = bucket.begin(); it != bucket.end(); ++it)
            {
                if (this->equaler(*it, key))
                {
                    bucket.erase(it);
                    --this->_size;
                    return true;
                }
            }
            return false;
        }
    };

} // namespace EgLab