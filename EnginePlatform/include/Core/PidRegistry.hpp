#pragma once

/**
 * @file PidRegistry.hpp
 * @brief 注册表 PID 读写封装
 * @details 把 "当前活动进程 PID" 写入 HKCU\Software\EngineLab，
 *          并借助 Monitor 周期刷新注册表中的 PID 缓存，使界面可以实时判断
 *          当前进程是否为注册表中记录的那个进程（用于多开场景下的高亮）。
 */

#include <atomic>

#include "Common/Return.hpp"
#include "Common/Singleton.hpp"

namespace EgLab::Platform
{
    class PidRegistry : public Common::Singleton<PidRegistry>
    {
    public:
        /// 注册表中没有有效 PID 时的取值。
        static constexpr unsigned long emptyProcessId = 0;

        /// 把当前进程 PID 写入注册表。
        Common::Return writeCurrentProcessId();

        /// 直接从注册表读取 PID（不加缓存，立即访问注册表）。
        Common::Return readProcessId(unsigned long& processId) const;

        /// 把注册表 PID 的刷新交给 Monitor 周期执行。
        void startMonitoring(unsigned int intervalMs = 300);

        /// 停止周期刷新，并从 Monitor 注销监控项。
        void stopMonitoring();

        /// 是否正在周期监控。
        bool isMonitoring() const;

        /// 最近一次监控读到的注册表 PID（0 表示无效或读取失败）。
        unsigned long registeredProcessId() const;

        /// 当前进程 PID。
        unsigned long currentProcessId() const;

        /// 当前进程是否就是注册表中记录的那个进程，界面据此高亮。
        bool isCurrentProcessRegistered() const;

        /// 最近一次注册表访问的错误码（Windows 下为 LSTATUS，0 表示成功）。
        long lastError() const;

        /// 立即刷新一次缓存（可用于写入后或手动触发）。
        void refresh();

    private:
        friend class Common::Singleton<PidRegistry>;

        PidRegistry();
        ~PidRegistry();

        /// Monitor 的回调入口，转调 refresh()。
        static void onMonitorTick(void* userData);

        mutable std::atomic<unsigned long> _registeredProcessId{emptyProcessId};
        mutable std::atomic<long> _lastError{0};
        std::atomic<bool> _monitoring{false};
    };
} // namespace EgLab::Platform
