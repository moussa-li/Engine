#pragma once

#include <stddef.h>

#include <atomic>
#include <cstdint>

#include "Common/DynamicArray.hpp"
#include "Common/String.hpp"

namespace EgLab::Common
{

    constexpr uint64_t RING_BUFFER_SIZE = 1024;
    constexpr uint64_t RING_BUFFER_MASK = RING_BUFFER_SIZE - 1;
    constexpr uint64_t MAX_CHANNELS = 16;
    constexpr size_t MAX_PAYLOAD_SIZE = 65536;

    enum class SlotState : uint32_t
    {
        EMPTY = 0,   // 空闲
        WRITING = 1, // 正在写入
        READY = 2,   // 写入完成，等待读取
        READING = 3  // 正在读取
    };

    struct MessageHeader
    {
        uint64_t timestamp;
        std::atomic<SlotState> state;
        uint32_t channelId;
        uint32_t payloadSize;
        uint32_t reserved;
    };

    class IMemoryBlock
    {
    public:
        virtual ~IMemoryBlock() = default;
        virtual void* getAddtess() = 0;
        virtual size_t getSize() = 0;
    };

    class LocalBlock : public IMemoryBlock
    {
    public:
    private:
        DynamicArray<uint8_t> _buffer;
    };

    struct MemoryControlBlock
    {
        alignas(64) std::atomic<uint64_t> writeIdx;
        alignas(64) std::atomic<uint64_t> readIdx;
        alignas(64) std::atomic<uint64_t> subscriptionMask;

        MessageHeader ringBuffer[RING_BUFFER_SIZE];
    };

    class CommonAPI MemoryQueue
    {
    public:
        MemoryQueue(const Common::String& name, bool createIfNotExist = true);
        ~MemoryQueue();

        MemoryQueue(const MemoryQueue&) = delete;
        MemoryQueue& operator=(const MemoryQueue&) = delete;

        bool push(uint32_t channelId, const void* data, uint32_t size);
        bool pop(uint32_t& channelId, void* outBuffer, uint32_t bufferSize,
                 uint32_t& outActualSize);

    private:
        Common::String _name;
        void* _platformHandle;
        void* _basePtr;
        size_t _size;
        bool _isOwner;
        MemoryControlBlock* _controlBlock;
    };

} // namespace EgLab::Common
