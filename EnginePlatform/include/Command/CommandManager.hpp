#pragma once

#include <type_traits>

#include "Command/CommandIds.hpp"
#include "Common/DynamicArray.hpp"
#include "Common/HashMap.hpp"
#include "Common/Protocol.hpp"
#include "Common/Singleton.hpp"
#include "Common/String.hpp"
#include "Common/UniquePtr.hpp"

namespace EgLab::Platform
{
    constexpr size_t MAX_PAYLOAD_SIZE = EgLab::Common::MAX_PAYLOAD_SIZE - sizeof(uint32_t) * 2;

    struct PacketPOD
    {
        uint32_t dataSize;
        uint8_t data[MAX_PAYLOAD_SIZE];
    };

    template <typename T>
    struct PacketTypeTraits
    {
        static constexpr bool isString = false;
        static constexpr bool isBool = false;
    };

    template <>
    struct PacketTypeTraits<bool>
    {
        static constexpr bool isString = false;
        static constexpr bool isBool = true;
    };

    template <>
    struct PacketTypeTraits<Common::String>
    {
        static constexpr bool isString = true;
        static constexpr bool isBool = false;
    };

    class EventPacket
    {
    public:
        template <typename T>
        void setData(const Common::String& key, T value)
        {
            if constexpr (PacketTypeTraits<T>::isString)
            {
                writeData(key, value);
            }
            else if constexpr (PacketTypeTraits<T>::isBool)
            {
                uint8_t v = value ? 1 : 0;
                writeData(key, &v, sizeof(uint8_t));
            }
            else
            {
                static_assert(std::is_trivially_copyable<T>::value,
                              "Type must be trivially copyable");
                writeData(key, &value, sizeof(T));
            }
        }

        template <typename T>
        T getData(const Common::String& key) const
        {
            if constexpr (PacketTypeTraits<T>::isString)
            {
                const uint8_t* lenPtr = findData(key, sizeof(uint32_t));
                uint32_t len;
                std::memcpy(&len, lenPtr, sizeof(uint32_t));
                const uint8_t* strPtr = lenPtr + sizeof(uint32_t);
                return Common::String(reinterpret_cast<const char*>(strPtr), len);
            }
            else if constexpr (PacketTypeTraits<T>::isBool)
            {
                const uint8_t* ptr = findData(key, sizeof(uint8_t));
                return (*ptr) != 0;
            }
            else
            {
                const uint8_t* ptr = findData(key, sizeof(T));
                T value;
                std::memcpy(&value, ptr, sizeof(T));
                return value;
            }
        }

        const Common::DynamicArray<uint8_t>& getRawBuffer() const
        {
            return _buffer;
        }

    private:
        void writeData(const Common::String& key, const void* data, size_t size)
        {
            size_t offset = _buffer.size();
            _indexMap[key] = offset;
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
            _buffer.pushBack(bytes, size);
        }

        void writeData(const Common::String& key, const Common::String& data)
        {
            size_t offset = _buffer.size();
            _indexMap[key] = offset;
            const uint32_t len = static_cast<uint32_t>(data.size() + 1);
            _buffer.pushBack((uint8_t*)&len, sizeof(uint32_t));
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data.c_str());
            _buffer.pushBack(bytes, data.size() + 1);
        }

        const uint8_t* findData(const Common::String& key, size_t expectedSize) const
        {
            auto it = _indexMap.find(key);
            if (it == _indexMap.end())
            {
                auto warn = "EventPacket: Key not found - " + key;
                throw std::runtime_error(warn.c_str());
            }
            if (it.data().second + expectedSize > _buffer.size())
            {
                auto warn = "EventPacket: Data out of bounds - " + key;
                throw std::runtime_error(warn.c_str());
            }
            return &_buffer[it.data().second];
        }

    private:
        Common::DynamicArray<uint8_t> _buffer;
        Common::HashMap<Common::String, size_t> _indexMap;
    };

    class CommandManager : public Common::Singleton<CommandManager>
    {
    public:
        EventId getId(const Common::String& name);

        void call(EventId, PacketPOD);
        void call(EventId, EventPacket);

    private:
        CommandManager();
        ~CommandManager();
        friend class Common::Singleton<CommandManager>;

        class Impl;
        Common::UniquePtr<Impl> _impl;
    };

} // namespace EgLab::Platform