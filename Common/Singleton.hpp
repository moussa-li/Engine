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