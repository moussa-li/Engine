/**
 * @file Singleton.hpp
 * @author Moussa-Li
 * @brief the singleton base, which derive this class could 
 *        implacement the simple singleton, not thread safely.
 * @date 2025-10-26
 */

#pragma once

namespace EgLab
{
    template<class Derived>
    class Singleton
    {
    public:
        static Derived& instance() {
            static Derived instance;
            return instance;
        }

        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;

    protected:
        Singleton() = default;
        ~Singleton() = default;
    };
}