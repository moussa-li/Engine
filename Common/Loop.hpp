#pragma once

#include <cstddef>
#include <type_traits>

namespace EgLab::Common
{
    template <typename T, size_t N>
    class ConstLoop
    {
    public:
        ConstLoop() = default;

        ConstLoop(const ConstLoop &other) = default;

        ConstLoop(ConstLoop &&other) noexcept = default;

        ConstLoop &operator=(const ConstLoop &other) = default;

        ConstLoop &operator=(ConstLoop &&other) noexcept = default;

        template <typename... Args,
                  typename = std::enable_if_t<sizeof...(Args) == N &&
                                              (std::is_convertible_v<Args, T> && ...)>>
        ConstLoop(Args... args) : _data{static_cast<T>(args)...}
        {
        }

        void next()
        {
            currentIdx++;
            currentIdx %= N;
        }

        void prev()
        {
            currentIdx--;
            currentIdx += N;
            currentIdx %= N;
        }

        T &current() const
        {
            return _data[currentIdx];
        }

        size_t getCurrentIdx() const
        {
            return currentIdx;
        }

        T get(size_t i) const
        {
            return _data[(i + currentIdx) % N];
        }

    private:
        T _data[N];

        size_t currentIdx = 0;
    };
} // namespace EgLab::Common