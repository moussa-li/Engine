#pragma once

namespace EgLab
{
    template <typename F, typename S>
    struct Pair
    {
        F first;
        S second;

        Pair() : first(), second()
        {
        }

        Pair(const F& f, const S& s) : first(f), second(s)
        {
        }

        Pair(const Pair& other) : first(other.first), second(other.second)
        {
        }

        Pair& operator=(const Pair& other)
        {
            if (this != &other)
            {
                first = other.first;
                second = other.second;
            }
            return *this;
        }

        bool operator==(const Pair& other) const
        {
            return first == other.first && second == other.second;
        }
    };

    template <typename F, typename S>
    Pair<F, S> make_pair(const F& f, const S& s)
    {
        return Pair<F, S>(f, s);
    }

} // namespace EgLab