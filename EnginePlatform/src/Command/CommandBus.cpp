#include "Command/CommandBus.hpp"

#include <Common/DynamicArray.hpp>
#include <queue>

namespace EgLab::Platform
{
    class CommandBus::Impl
    {
    public:
        Common::DynamicArray<std::thread> _workers;
        std::queue<Common::SharedPtr<ICommand>> _commandQueue;

        std::mutex _mutex;
        std::condition_variable _cv;
        bool _stop = false;
    };

    CommandBus::CommandBus(size_t threadCount) : _impl(EgLab::Common::makeUnique<Impl>())
    {
        for (size_t i = 0; i < threadCount; i++)
        {
            _impl->_workers.pushBack(std::thread([this] {
                while (true)
                {
                    Common::SharedPtr<ICommand> command;
                    {
                        std::unique_lock<std::mutex> lock(_impl->_mutex);
                        _impl->_cv.wait(
                            lock, [this] { return !_impl->_commandQueue.empty() || _impl->_stop; });
                        if (_impl->_stop && _impl->_commandQueue.empty()) return;
                        command = Common::move(_impl->_commandQueue.front());
                        _impl->_commandQueue.pop();

                        if (command) command->exec();
                    }
                }
            }));
        }
    }

    CommandBus::~CommandBus()
    {
        {
            std::unique_lock<std::mutex> lock(_impl->_mutex);
            _impl->_stop = true;
        }
        _impl->_cv.notify_all();
        for (std::thread& worker : _impl->_workers)
        {
            if (worker.joinable()) worker.join();
        }
    }

    Common::Return CommandBus::dispatch(Common::SharedPtr<ICommand> command)
    {
        {
            std::unique_lock<std::mutex> lock(_impl->_mutex);
            _impl->_commandQueue.push(Common::move(command));
        }
        _impl->_cv.notify_one();
        return Common::Return::Succeed;
    }

} // namespace EgLab::Platform
