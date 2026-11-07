#pragma once

/**
 * @file Monitor.hpp
 * @brief 通用的周期监控器
 * @details 以独立线程按固定节拍轮询已注册的监控项。监控项（WatchItem）由名称、
 *          回调函数、回调上下文以及自定义轮询间隔组成，回调在监控线程中被调用，
 *          因此回调内部不要直接操作 ImGui 等非线程安全资源，只更新缓存的原子量即可。
 */

#include <condition_variable>
#include <mutex>
#include <thread>

#include "Common/DynamicArray.hpp"
#include "Common/Singleton.hpp"
#include "Common/String.hpp"

namespace EgLab::Platform
{
    class Monitor : public Common::Singleton<Monitor>
    {
    public:
        /// 监控回调。为了不引入 std::function，这里使用函数指针 + 用户上下文。
        using Callback = void (*)(void* userData);

        /// 单个监控项。
        struct WatchItem
        {
            Common::String name;               // 监控项名称，用于增删改查
            Callback callback = nullptr;       // 到期时执行的回调
            void* userData = nullptr;          // 回调上下文
            unsigned int intervalMs = 0;       // 0 表示使用监控器的默认节拍间隔
            unsigned long long nextTickMs = 0; // 下一次到期的时刻（毫秒）
            bool active = false;               // 是否启用
        };

        /// 启动监控线程。tickIntervalMs 为默认节拍间隔。
        void start(unsigned int tickIntervalMs = 200);

        /// 停止监控线程并等待其退出。
        void stop();

        /// 监控线程是否正在运行。
        bool isRunning() const;

        /// 注册（或更新同名）监控项。intervalMs 为 0 时使用默认节拍间隔。
        void addWatch(const Common::String& name, Callback callback, void* userData,
                      unsigned int intervalMs = 0);

        /// 停用监控项（惰性移除，可在线程运行时调用）。
        void removeWatch(const Common::String& name);

        /// 让监控项在下一个节拍立即执行一次。
        void triggerWatch(const Common::String& name);

        /// 默认节拍间隔（毫秒）。
        unsigned int tickIntervalMs() const;

        /// 已完成的节拍数量，可用于调试。
        unsigned long long tickCount() const;

    private:
        friend class Common::Singleton<Monitor>;

        Monitor();
        ~Monitor();

        void workerLoop();
        static unsigned long long nowMs();

        mutable std::mutex _mutex;
        std::condition_variable _condition;
        std::thread _worker;

        Common::DynamicArray<WatchItem> _watches;
        unsigned int _tickIntervalMs{200};
        unsigned long long _tickCount{0};
        bool _running{false};
        bool _stopping{false};
    };
} // namespace EgLab::Platform
