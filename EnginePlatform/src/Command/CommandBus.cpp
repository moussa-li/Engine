#include "Command/CommandBus.hpp"

#include <Common/DynamicArray.hpp>
#include <condition_variable>
#include <queue>
#include <thread>

namespace EgLab::Platform
{
    class CommandBus::Impl
    {
    public:
        Common::DynamicArray<std::thread> _workers;
        std::queue<Common::SharedPtr<ICommand>> _commandQueue;

        std::mutex _mutex;
        std::condition_variable _cv;
        std::condition_variable _idleCv;
        size_t _pendingCount{0};
        bool _pushAllowed{true};
        bool _stop = false;
        bool _discardPending = false;
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
                        if (_impl->_stop &&
                            (_impl->_discardPending || _impl->_commandQueue.empty()))
                        {
                            return;
                        }
                        command = Common::move(_impl->_commandQueue.front());
                        _impl->_commandQueue.pop();
                    }
                    if (command)
                    {
                        try
                        {
                            command->exec();
                        }
                        catch (...)
                        {
                        }

                        bool idle = false;
                        {
                            std::lock_guard<std::mutex> lock(_impl->_mutex);
                            idle = --_impl->_pendingCount == 0;
                        }
                        if (idle)
                        {
                            _impl->_idleCv.notify_all();
                        }
                    }
                }
            }));
        }
    }

    CommandBus::~CommandBus()
    {
        stop();
    }

    void CommandBus::stop(bool discardPending)
    {
        std::queue<Common::SharedPtr<ICommand>> discardedCommands;
        {
            std::lock_guard<std::mutex> lock(_impl->_mutex);
            _impl->_stop = true;
            _impl->_discardPending = _impl->_discardPending || discardPending;
            if (_impl->_discardPending)
            {
                while (!_impl->_commandQueue.empty())
                {
                    _impl->_commandQueue.pop();
                    --_impl->_pendingCount;
                }
                _impl->_commandQueue.swap(discardedCommands);
            }
        }
        _impl->_cv.notify_all();
        _impl->_idleCv.notify_all();
        for (std::thread& worker : _impl->_workers)
        {
            if (worker.joinable()) worker.join();
        }
    }

    Common::Return CommandBus::dispatch(Common::SharedPtr<ICommand> command)
    {
        return dispatch(Common::move(command), false);
    }

    Common::Return CommandBus::dispatch(Common::SharedPtr<ICommand> command,
                                        bool waitForPushAllowed)
    {
        {
            std::unique_lock<std::mutex> lock(_impl->_mutex);
            if (waitForPushAllowed)
            {
                _impl->_idleCv.wait(lock, [this] {
                    return _impl->_stop || (_impl->_pushAllowed && _impl->_pendingCount == 0);
                });
            }
            if (_impl->_stop)
            {
                return Common::Return::Failed;
            }
            _impl->_commandQueue.push(Common::move(command));
            ++_impl->_pendingCount;
        }
        _impl->_cv.notify_one();
        return Common::Return::Succeed;
    }

    void CommandBus::setPushAllowed(bool allowed)
    {
        {
            std::lock_guard<std::mutex> lock(_impl->_mutex);
            _impl->_pushAllowed = allowed;
        }
        _impl->_idleCv.notify_all();
    }

    void CommandBus::waitIdle()
    {
        std::unique_lock<std::mutex> lock(_impl->_mutex);
        _impl->_idleCv.wait(lock, [this] { return _impl->_pendingCount == 0; });
    }

} // namespace EgLab::Platform
