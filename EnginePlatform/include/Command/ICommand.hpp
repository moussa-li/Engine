#pragma once

#include "Command/CommandParam.hpp"
#include "Common/Return.hpp"
#include "Common/Utils.hpp"


namespace EgLab::Platform
{
    class ICommand
    {
    public:
        virtual ~ICommand() = default;

        virtual Common::Return exec() = 0;
    };

    template <typename T>
    class CommandBase : public ICommand
    {
    public:
        explicit CommandBase(const T& params) : _params(params)
        {
        }
        explicit CommandBase(T&& params) : _params(Common::move(params))
        {
        }

        const T& getParams() const
        {
            return _params;
        }

    protected:
        T _params;
    };

} // namespace EgLab::Platform