#include "Core/Monitor.hpp"

#include <chrono>

namespace EgLab::Platform
{
    namespace
    {
        /// 计算下一次需要唤醒监控线程的时刻（毫秒）。
        unsigned long long computeDeadline(const Common::DynamicArray<Monitor::WatchItem>& watches,
                                           unsigned long long fallback)
        {
            unsigned long long deadline = fallback;
            for (size_t index = 0; index < watches.size(); ++index)
            {
                const Monitor::WatchItem& watch = watches[index];
                if (watch.active == false)
                {
                    continue;
                }
                if (watch.nextTickMs < deadline)
                {
                    deadline = watch.nextTickMs;
                }
            }
            return deadline;
        }
    } // namespace

    Monitor::Monitor()
    {
    }

    Monitor::~Monitor()
    {
        stop();
    }

    void Monitor::start(unsigned int tickIntervalMs)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_running)
        {
            return;
        }

        if (tickIntervalMs > 0)
        {
            _tickIntervalMs = tickIntervalMs;
        }

        _stopping = false;
        _running = true;
        _tickCount = 0;

        // 已注册的监控项在启动后立即执行一次，避免等待第一个节拍。
        const unsigned long long now = nowMs();
        for (size_t index = 0; index < _watches.size(); ++index)
        {
            if (_watches[index].active)
            {
                _watches[index].nextTickMs = now;
            }
        }

        _worker = std::thread(&Monitor::workerLoop, this);
    }

    void Monitor::stop()
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_running == false)
            {
                return;
            }
            _stopping = true;
        }

        _condition.notify_all();

        if (_worker.joinable())
        {
            _worker.join();
        }

        std::lock_guard<std::mutex> lock(_mutex);
        _running = false;
        _stopping = false;
    }

    bool Monitor::isRunning() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _running;
    }

    void Monitor::addWatch(const Common::String& name, Callback callback, void* userData,
                           unsigned int intervalMs)
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);

            // 同名监控项直接复用，避免重复注册。
            for (size_t index = 0; index < _watches.size(); ++index)
            {
                if (_watches[index].name == name)
                {
                    WatchItem& watch = _watches[index];
                    watch.callback = callback;
                    watch.userData = userData;
                    watch.intervalMs = intervalMs;
                    watch.active = true;
                    watch.nextTickMs = nowMs();
                    return;
                }
            }

            WatchItem watch;
            watch.name = name;
            watch.callback = callback;
            watch.userData = userData;
            watch.intervalMs = intervalMs;
            watch.active = true;
            watch.nextTickMs = nowMs();
            _watches.pushBack(watch);
        }

        // 新监控项可能比当前等待时长更早到期，唤醒监控线程重新计算节拍。
        _condition.notify_all();
    }

    void Monitor::removeWatch(const Common::String& name)
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            for (size_t index = 0; index < _watches.size(); ++index)
            {
                if (_watches[index].name == name)
                {
                    _watches[index].active = false;
                    break;
                }
            }
        }
        _condition.notify_all();
    }

    void Monitor::triggerWatch(const Common::String& name)
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            for (size_t index = 0; index < _watches.size(); ++index)
            {
                if (_watches[index].name == name)
                {
                    _watches[index].nextTickMs = nowMs();
                    break;
                }
            }
        }
        _condition.notify_all();
    }

    unsigned int Monitor::tickIntervalMs() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _tickIntervalMs;
    }

    unsigned long long Monitor::tickCount() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _tickCount;
    }

    unsigned long long Monitor::nowMs()
    {
        const auto now = std::chrono::steady_clock::now().time_since_epoch();
        return static_cast<unsigned long long>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
    }

    void Monitor::workerLoop()
    {
        while (true)
        {
            Common::DynamicArray<WatchItem> due;

            {
                std::unique_lock<std::mutex> lock(_mutex);
                if (_stopping)
                {
                    return;
                }

                const unsigned long long now = nowMs();
                for (size_t index = 0; index < _watches.size(); ++index)
                {
                    WatchItem& watch = _watches[index];
                    if (watch.active == false || watch.nextTickMs > now)
                    {
                        continue;
                    }

                    due.pushBack(watch);
                    const unsigned int interval =
                        watch.intervalMs > 0 ? watch.intervalMs : _tickIntervalMs;
                    watch.nextTickMs = now + interval;
                }

                if (due.empty() == false)
                {
                    ++_tickCount;
                }

                // 回调在锁外执行，避免回调内部再次访问监控器造成死锁。
                lock.unlock();
                for (size_t index = 0; index < due.size(); ++index)
                {
                    if (due[index].callback != nullptr)
                    {
                        due[index].callback(due[index].userData);
                    }
                }
                lock.lock();

                if (_stopping)
                {
                    return;
                }

                const unsigned long long waitNow = nowMs();
                const unsigned long long deadline =
                    computeDeadline(_watches, waitNow + _tickIntervalMs);
                if (deadline > waitNow)
                {
                    _condition.wait_for(lock, std::chrono::milliseconds(deadline - waitNow));
                }
            }
        }
    }
} // namespace EgLab::Platform
