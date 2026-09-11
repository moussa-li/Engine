#include "Protocol.hpp"
#ifdef _WIN32

#include <windows.h>

#include <stdexcept>

namespace EgLab::Common
{
    constexpr size_t SHM_SIZE = sizeof(MemoryControlBlock) +
                                (RING_BUFFER_SIZE * (sizeof(MessageHeader) + MAX_PAYLOAD_SIZE));

    MemoryQueue::MemoryQueue(const Common::String& name, bool createIfNotExist)
        : _name(name),
          _platformHandle(nullptr),
          _basePtr(nullptr),
          _size(SHM_SIZE),
          _isOwner(false),
          _controlBlock(nullptr)
    {
        HANDLE hMapFile = nullptr;
        if (createIfNotExist)
        {
            hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
                                          static_cast<DWORD>(_size), _name.c_str());
            if (hMapFile == NULL) throw std::runtime_error("CreateFileMapping failed");
            _isOwner = (GetLastError() != ERROR_ALREADY_EXISTS);
        }
        else
        {
            hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, _name.c_str());
            if (hMapFile == NULL) throw std::runtime_error("CreateFileMapping failed");
        }
        _platformHandle = hMapFile;
        _basePtr = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, _size);
        if (_basePtr == nullptr)
        {
            CloseHandle(hMapFile);
            throw std::runtime_error("MapViewOfFile failed");
        }

        _controlBlock = static_cast<MemoryControlBlock*>(_basePtr);
        if (_isOwner)
        {
            new (&_controlBlock->writeIdx) std::atomic<uint64_t>(0);
            new (&_controlBlock->readIdx) std::atomic<uint64_t>(0);

            uint8_t* dataZone = static_cast<uint8_t*>(_basePtr) + sizeof(MemoryControlBlock);
            for (uint64_t i = 0; i < RING_BUFFER_SIZE; i++)
            {
                MessageHeader* header = reinterpret_cast<MessageHeader*>(
                    dataZone + i * (sizeof(MessageHeader) + MAX_PAYLOAD_SIZE));
                new (&header->state) std::atomic<SlotState>(SlotState::EMPTY);
            }
        }
    }

    MemoryQueue::~MemoryQueue()
    {
        if (_basePtr) UnmapViewOfFile(_basePtr);
        if (_platformHandle) CloseHandle(static_cast<HANDLE>(_platformHandle));
    }

    bool MemoryQueue::push(uint32_t channelId, const void* data, uint32_t size)
    {
        if (size > MAX_PAYLOAD_SIZE) return false;
        uint64_t currentWrite = _controlBlock->writeIdx.load(std::memory_order_relaxed);
        uint64_t currentRead = _controlBlock->readIdx.load(std::memory_order_acquire);
        if ((currentWrite - currentRead) >= RING_BUFFER_SIZE) return false;

        uint64_t slotIdx = currentWrite & RING_BUFFER_MASK;
        uint8_t* dataZone = static_cast<uint8_t*>(_basePtr) + sizeof(MemoryControlBlock);
        uint8_t* slotPtr = dataZone + (slotIdx * (sizeof(MessageHeader) + MAX_PAYLOAD_SIZE));

        MessageHeader* header = reinterpret_cast<MessageHeader*>(slotPtr);
        SlotState expected = SlotState::EMPTY;
        while (!header->state.compare_exchange_weak(expected, SlotState::WRITING,
                                                    std::memory_order_acquire))
        {
            if (expected != SlotState::EMPTY) return false;
            expected = SlotState::EMPTY;
        }

        header->timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                std::chrono::system_clock::now().time_since_epoch())
                                .count();
        header->channelId = channelId;
        header->payloadSize = size;
        if (data && size > 0) std::memcpy(slotPtr + sizeof(MessageHeader), data, size);

        header->state.store(SlotState::READY, std::memory_order_release);

        _controlBlock->writeIdx.store(currentWrite + 1, std::memory_order_release);
        return true;
    }

    bool MemoryQueue::pop(uint32_t& channelId, void* outBuffer, uint32_t bufferSize,
                          uint32_t& outActualSize)
    {
        uint64_t currentWrite = _controlBlock->writeIdx.load(std::memory_order_acquire);
        uint64_t currentRead = _controlBlock->readIdx.load(std::memory_order_relaxed);

        // 队列空检查
        if (currentRead >= currentWrite) return false;

        uint64_t slotIdx = currentRead & RING_BUFFER_MASK;
        uint8_t* dataZone = static_cast<uint8_t*>(_basePtr) + sizeof(MemoryControlBlock);
        uint8_t* slotPtr = dataZone + (slotIdx * (sizeof(MessageHeader) + MAX_PAYLOAD_SIZE));

        MessageHeader* header = reinterpret_cast<MessageHeader*>(slotPtr);

        // 1. 尝试将状态从 READY 转换为 READING
        SlotState expected = SlotState::READY;
        if (!header->state.compare_exchange_strong(expected, SlotState::READING,
                                                   std::memory_order_acquire))
        {
            return false; // 还没准备好，或者状态不对
        }

        // 2. 读取数据
        channelId = header->channelId;
        outActualSize = header->payloadSize;

        if (outBuffer && outActualSize > 0)
        {
            if (outActualSize > bufferSize) outActualSize = bufferSize;
            std::memcpy(outBuffer, slotPtr + sizeof(MessageHeader), outActualSize);
        }

        // 3. 将状态重置为 EMPTY，供下一轮写入
        header->state.store(SlotState::EMPTY, std::memory_order_release);

        // 4. 推进读指针
        _controlBlock->readIdx.store(currentRead + 1, std::memory_order_release);
        return true;
    }

} // namespace EgLab::Common

#else

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdexcept>

namespace EgLab::Common
{

}
#endif