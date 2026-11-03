#pragma once
#include "Common/String.hpp"

namespace EgLab::Common
{
    class Exception
    {
    public:
        virtual ~Exception() = default;

        virtual const char* what() const noexcept = 0;
    };

    class OutOfMemoryException : public Exception
    {
    public:
        OutOfMemoryException() = default;
        OutOfMemoryException(const OutOfMemoryException&) = default;
        OutOfMemoryException(String message) : _msg(message)
        {
        }

        const char* what() const noexcept override
        {
            String msg = "[ Out of memory ] : " + _msg;
            return msg.c_str();
        }

    private:
        String _msg;
    };
} // namespace EgLab::Common