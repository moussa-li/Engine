#pragma once

namespace EgLab
{
    template <typename T>
    class Property
    {
    public:
        Property() = default;
        Property(const T& val) : value(val)
        {
        }

        const T& get() const
        {
            return value;
        }

        void set(const T& val)
        {
            value = val;
        }

        Property<T>& operator=(const T& val)
        {
            value = val;
            return *this;
        }

        operator T() const
        {
            return value;
        }

    private:
        T value;
    };
} // namespace EgLab