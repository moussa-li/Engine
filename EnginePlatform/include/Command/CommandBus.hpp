#pragma once

#include <stddef.h>

#include <thread>

#include "Command/ICommand.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/UniquePtr.hpp"

namespace EgLab::Platform
{
    class CommandBus
    {
    public:
        CommandBus(size_t threadCount = std::thread::hardware_concurrency());

        ~CommandBus();

        Common::Return dispatch(Common::SharedPtr<ICommand> command);

    private:
        class Impl;
        Common::UniquePtr<Impl> _impl;
    };

} // namespace EgLab::Platform