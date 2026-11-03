#pragma once

namespace EgLab::RE
{
    /// Stores a value and provides property-style access and assignment.
    template <typename T>
    class Property
    {
    public:
        /// @brief Creates a property with a default-initialized value.
        Property() = default;
        /// @brief Creates a property with an initial value.
        /// @param val Initial value.
        Property(const T& val) : value(val)
        {
        }

        /// @brief Returns the stored value.
        /// @return A constant reference to the stored value.
        const T& get() const
        {
            return value;
        }

        /// @brief Replaces the stored value.
        /// @param val New value.
        void set(const T& val)
        {
            value = val;
        }

        /// @brief Assigns a value to the property.
        /// @param val New value.
        /// @return A reference to this property.
        Property<T>& operator=(const T& val)
        {
            value = val;
            return *this;
        }

        /// @brief Converts the property to its stored value.
        /// @return A copy of the stored value.
        operator T() const
        {
            return value;
        }

    private:
        T value;
    };
} // namespace EgLab::RE