#include "Core/PidRegistry.hpp"

#include "Core/Monitor.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

namespace EgLab::Platform
{
    namespace
    {
#ifdef _WIN32
        constexpr const wchar_t* REGISTRY_PATH = L"Software\\EngineLab";
        constexpr const wchar_t* REGISTRY_VALUE_NAME = L"CURRENT_ENGINELAB_PROCESSID";
#endif
        constexpr const char* MONITOR_WATCH_NAME = "Core.PidRegistry";
    } // namespace

    PidRegistry::PidRegistry()
    {
    }

    PidRegistry::~PidRegistry()
    {
        // 正常情况下 Application 会主动调用 stopMonitoring()，
        // 这里的判断保证单例析构时不会再去触碰已经销毁的 Monitor。
        stopMonitoring();
    }

    unsigned long PidRegistry::currentProcessId() const
    {
#ifdef _WIN32
        return static_cast<unsigned long>(GetCurrentProcessId());
#else
        return emptyProcessId;
#endif
    }

    Common::Return PidRegistry::writeCurrentProcessId()
    {
#ifdef _WIN32
        HKEY key = nullptr;
        const LSTATUS openStatus =
            RegCreateKeyExW(HKEY_CURRENT_USER, REGISTRY_PATH, 0, nullptr, REG_OPTION_NON_VOLATILE,
                            KEY_SET_VALUE, nullptr, &key, nullptr);
        if (openStatus != ERROR_SUCCESS)
        {
            _lastError.store(static_cast<long>(openStatus));
            return Common::Return::Failed;
        }

        const DWORD processId = GetCurrentProcessId();
        const LSTATUS writeStatus =
            RegSetValueExW(key, REGISTRY_VALUE_NAME, 0, REG_DWORD,
                           reinterpret_cast<const BYTE*>(&processId), sizeof(processId));
        const LSTATUS closeStatus = RegCloseKey(key);
        const LSTATUS status = writeStatus == ERROR_SUCCESS ? closeStatus : writeStatus;
        _lastError.store(static_cast<long>(status));
        if (status != ERROR_SUCCESS)
        {
            return Common::Return::Failed;
        }

        // 写入成功后立即更新缓存，界面下一帧即可高亮。
        _registeredProcessId.store(static_cast<unsigned long>(processId));
        return Common::Return::Succeed;
#else
        return Common::Return::NotImplacementYet;
#endif
    }

    Common::Return PidRegistry::readProcessId(unsigned long& processId) const
    {
        processId = emptyProcessId;

#ifdef _WIN32
        HKEY key = nullptr;
        const LSTATUS openStatus =
            RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_PATH, 0, KEY_QUERY_VALUE, &key);
        if (openStatus != ERROR_SUCCESS)
        {
            _lastError.store(static_cast<long>(openStatus));
            return (openStatus == ERROR_FILE_NOT_FOUND || openStatus == ERROR_PATH_NOT_FOUND)
                       ? Common::Return::FileNotFound
                       : Common::Return::Failed;
        }

        DWORD value = 0;
        DWORD valueSize = static_cast<DWORD>(sizeof(value));
        DWORD valueType = 0;
        const LSTATUS readStatus = RegQueryValueExW(key, REGISTRY_VALUE_NAME, nullptr, &valueType,
                                                    reinterpret_cast<BYTE*>(&value), &valueSize);
        RegCloseKey(key);

        _lastError.store(static_cast<long>(readStatus));
        if (readStatus != ERROR_SUCCESS)
        {
            return Common::Return::Failed;
        }
        if (valueType != REG_DWORD || static_cast<size_t>(valueSize) != sizeof(DWORD))
        {
            return Common::Return::BadInput;
        }

        processId = static_cast<unsigned long>(value);
        return Common::Return::Succeed;
#else
        (void)processId;
        return Common::Return::NotImplacementYet;
#endif
    }

    void PidRegistry::startMonitoring(unsigned int intervalMs)
    {
        if (_monitoring.exchange(true))
        {
            return;
        }

        Monitor& monitor = Monitor::instance();
        monitor.addWatch(MONITOR_WATCH_NAME, &PidRegistry::onMonitorTick, this, intervalMs);
        if (monitor.isRunning() == false)
        {
            monitor.start(intervalMs);
        }

        // 不等待第一个节拍，先同步刷新一次。
        refresh();
    }

    void PidRegistry::stopMonitoring()
    {
        if (_monitoring.exchange(false) == false)
        {
            return;
        }

        Monitor::instance().removeWatch(MONITOR_WATCH_NAME);
    }

    bool PidRegistry::isMonitoring() const
    {
        return _monitoring.load();
    }

    void PidRegistry::onMonitorTick(void* userData)
    {
        if (userData == nullptr)
        {
            return;
        }
        static_cast<PidRegistry*>(userData)->refresh();
    }

    void PidRegistry::refresh()
    {
        unsigned long processId = emptyProcessId;
        if (readProcessId(processId) == Common::Return::Succeed)
        {
            _registeredProcessId.store(processId);
        }
        else
        {
            _registeredProcessId.store(emptyProcessId);
        }
    }

    unsigned long PidRegistry::registeredProcessId() const
    {
        return _registeredProcessId.load();
    }

    bool PidRegistry::isCurrentProcessRegistered() const
    {
        const unsigned long registered = _registeredProcessId.load();
        return registered != emptyProcessId && registered == currentProcessId();
    }

    long PidRegistry::lastError() const
    {
        return _lastError.load();
    }
} // namespace EgLab::Platform
