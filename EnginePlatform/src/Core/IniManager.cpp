#include "Core/IniManager.hpp"

#include <cctype>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <utility>

namespace EgLab::Platform
{
    namespace
    {
        std::string trim(const std::string &value)
        {
            const auto first = value.find_first_not_of(" \t\r\n");
            if (first == std::string::npos)
            {
                return {};
            }
            const auto last = value.find_last_not_of(" \t\r\n");
            return value.substr(first, last - first + 1);
        }

        template <typename T>
        T parseNumber(const Common::String &value, T defaultValue)
        {
            std::istringstream input(value.c_str());
            T parsed{};
            input >> parsed;
            if (input.fail())
            {
                return defaultValue;
            }
            input >> std::ws;
            return input.eof() ? parsed : defaultValue;
        }

        template <typename T>
        std::string formatNumber(T value)
        {
            std::ostringstream output;
            output << std::setprecision(std::numeric_limits<T>::max_digits10) << value;
            return output.str();
        }
    } // namespace

    IniManager::IniManager() : _filePath("EgPlatform.ini")
    {
    }

    IniManager::~IniManager()
    {
        shutdown();
    }

    void IniManager::start()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_started)
        {
            return;
        }

        loadFromDisk();
        _stopping = false;
        _started = true;
        _worker = std::thread(&IniManager::workerLoop, this);
    }

    void IniManager::shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_started)
            {
                _stopping = true;
                _condition.notify_all();
            }
        }

        if (_worker.joinable())
        {
            _worker.join();
        }

        flush();

        std::lock_guard<std::mutex> lock(_mutex);
        _started = false;
        _stopping = false;
    }

    bool IniManager::flush()
    {
        std::lock_guard<std::mutex> saveLock(_saveMutex);
        while (true)
        {
            std::vector<Common::Pair<Common::String, Common::String>> snapshot;
            unsigned long long revision = 0;
            {
                std::lock_guard<std::mutex> lock(_mutex);
                if (!_dirty)
                {
                    return true;
                }
                for (const auto &entry : _values)
                {
                    snapshot.emplace_back(entry.first, entry.second);
                }
                revision = _revision;
            }

            if (!saveSnapshot(snapshot))
            {
                return false;
            }

            std::lock_guard<std::mutex> lock(_mutex);
            if (_revision == revision)
            {
                _dirty = false;
                return true;
            }
        }
    }

    void IniManager::setValue(const Common::String &key, const Common::String &value)
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            auto iter = _values.find(key);
            if (iter != _values.end() && iter.data().second == value)
            {
                return;
            }
            _values[key] = value;
            _dirty = true;
            ++_revision;
        }
        _condition.notify_one();
    }

    Common::String IniManager::getValue(const Common::String &key,
                                        const Common::String &defaultValue) const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_values.empty())
        {
            return defaultValue;
        }
        auto iter = _values.find(key);
        return iter == _values.end() ? defaultValue : iter.data().second;
    }

    void IniManager::setInt(const Common::String &key, int value)
    {
        const auto formatted = std::to_string(value);
        setValue(key, Common::String(formatted.c_str()));
    }

    void IniManager::setFloat(const Common::String &key, float value)
    {
        const auto formatted = formatNumber(value);
        setValue(key, Common::String(formatted.c_str()));
    }

    void IniManager::setDouble(const Common::String &key, double value)
    {
        const auto formatted = formatNumber(value);
        setValue(key, Common::String(formatted.c_str()));
    }

    void IniManager::setBool(const Common::String &key, bool value)
    {
        setValue(key, Common::String(value ? "true" : "false"));
    }

    int IniManager::getInt(const Common::String &key, int defaultValue) const
    {
        return parseNumber(getValue(key), defaultValue);
    }

    float IniManager::getFloat(const Common::String &key, float defaultValue) const
    {
        return parseNumber(getValue(key), defaultValue);
    }

    double IniManager::getDouble(const Common::String &key, double defaultValue) const
    {
        return parseNumber(getValue(key), defaultValue);
    }

    bool IniManager::getBool(const Common::String &key, bool defaultValue) const
    {
        const auto value = getValue(key);
        if (value == "true" || value == "1")
        {
            return true;
        }
        if (value == "false" || value == "0")
        {
            return false;
        }
        return defaultValue;
    }

    void IniManager::loadFromDisk()
    {
        std::ifstream input(_filePath);
        bool migratedLegacyFile = false;
        if (!input)
        {
            input.clear();
            input.open("EgPlatfrom.ini");
            migratedLegacyFile = static_cast<bool>(input);
        }

        std::string line;
        while (std::getline(input, line))
        {
            const auto trimmed = trim(line);
            if (trimmed.empty() || trimmed.front() == '#' || trimmed.front() == ';')
            {
                continue;
            }

            const auto separator = trimmed.find('=');
            if (separator == std::string::npos)
            {
                continue;
            }

            const auto key = trim(trimmed.substr(0, separator));
            const auto value = trim(trimmed.substr(separator + 1));
            if (!key.empty())
            {
                _values[Common::String(key.c_str())] = Common::String(value.c_str());
            }
        }

        if (migratedLegacyFile)
        {
            _dirty = true;
            ++_revision;
        }
    }

    bool IniManager::saveIfDirty()
    {
        std::lock_guard<std::mutex> saveLock(_saveMutex);
        std::vector<Common::Pair<Common::String, Common::String>> snapshot;
        unsigned long long revision = 0;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (!_dirty)
            {
                return true;
            }
            for (const auto &entry : _values)
            {
                snapshot.emplace_back(entry.first, entry.second);
            }
            revision = _revision;
        }

        if (!saveSnapshot(snapshot))
        {
            return false;
        }

        std::lock_guard<std::mutex> lock(_mutex);
        if (_revision == revision)
        {
            _dirty = false;
        }
        return true;
    }

    bool IniManager::saveSnapshot(
        const std::vector<Common::Pair<Common::String, Common::String>> &values) const
    {
        std::ofstream output(_filePath, std::ios::trunc);
        if (!output)
        {
            return false;
        }

        for (const auto &entry : values)
        {
            output << entry.first.c_str() << '=' << entry.second.c_str() << '\n';
        }
        output.flush();
        return output.good();
    }

    void IniManager::workerLoop()
    {
        while (true)
        {
            bool stopping = false;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _condition.wait_for(lock, std::chrono::seconds(1));
                stopping = _stopping;
            }

            saveIfDirty();
            if (stopping)
            {
                return;
            }
        }
    }
} // namespace EgLab::Platform