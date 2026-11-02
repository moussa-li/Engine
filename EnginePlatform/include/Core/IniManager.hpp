#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "Common/HashMap.hpp"
#include "Common/Pair.hpp"
#include "Common/Singleton.hpp"
#include "Common/String.hpp"

namespace EgLab::Platform
{
    class IniManager : public Common::Singleton<IniManager>
    {
    public:
        void start();
        void shutdown();
        bool flush();

        void setValue(const Common::String &key, const Common::String &value);
        Common::String getValue(const Common::String &key,
                                const Common::String &defaultValue = Common::String()) const;

        void setInt(const Common::String &key, int value);
        void setFloat(const Common::String &key, float value);
        void setDouble(const Common::String &key, double value);
        void setBool(const Common::String &key, bool value);

        int getInt(const Common::String &key, int defaultValue) const;
        float getFloat(const Common::String &key, float defaultValue) const;
        double getDouble(const Common::String &key, double defaultValue) const;
        bool getBool(const Common::String &key, bool defaultValue) const;

    private:
        friend class Common::Singleton<IniManager>;

        IniManager();
        ~IniManager();

        void loadFromDisk();
        bool saveIfDirty();
        bool saveSnapshot(
            const std::vector<Common::Pair<Common::String, Common::String>> &values) const;
        void workerLoop();

        const std::string _filePath;
        mutable std::mutex _mutex;
        std::mutex _saveMutex;
        std::condition_variable _condition;
        Common::HashMap<Common::String, Common::String> _values;
        std::thread _worker;
        bool _dirty{false};
        bool _started{false};
        bool _stopping{false};
        unsigned long long _revision{0};
    };
} // namespace EgLab::Platform