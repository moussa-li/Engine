#include "Protocol.hpp"
#ifdef _WIN32

#include <windows.h>

#ifdef READING
#undef READING
#endif

#include <stdexcept>

namespace EgLab::Common
{
    namespace
    {
        constexpr uint32_t fragmentMarker = 0x46524147;

        struct FragmentHeader
        {
            uint64_t messageId;
            uint32_t totalSize;
            uint32_t offset;
        };

        constexpr uint32_t fragmentDataSize =
            static_cast<uint32_t>(MAX_PAYLOAD_SIZE - sizeof(FragmentHeader));
        constexpr uint64_t maxFragmentedMessageSize = fragmentDataSize * RING_BUFFER_SIZE;
        std::atomic<uint64_t> nextFragmentMessageId{1};
    } // namespace

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
        auto reserveSlots = [this](uint64_t count, uint64_t& firstSlot) {
            uint64_t currentWrite = _controlBlock->writeIdx.load(std::memory_order_relaxed);
            while (true)
            {
                const uint64_t currentRead = _controlBlock->readIdx.load(std::memory_order_acquire);
                if (currentWrite - currentRead + count > RING_BUFFER_SIZE) return false;
                if (_controlBlock->writeIdx.compare_exchange_weak(
                        currentWrite, currentWrite + count, std::memory_order_acq_rel,
                        std::memory_order_relaxed))
                {
                    firstSlot = currentWrite;
                    return true;
                }
            }
        };

        auto writePacket = [&](uint64_t reservedSlot, const void* packetData, uint32_t packetSize,
                               uint32_t marker) {
            const uint64_t slotIdx = reservedSlot & RING_BUFFER_MASK;
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
            header->payloadSize = packetSize;
            header->reserved = marker;
            if (packetData && packetSize > 0)
            {
                std::memcpy(slotPtr + sizeof(MessageHeader), packetData, packetSize);
            }

            header->state.store(SlotState::READY, std::memory_order_release);
            return true;
        };

        if (size <= MAX_PAYLOAD_SIZE)
        {
            uint64_t firstSlot = 0;
            if (!reserveSlots(1, firstSlot)) return false;
            return writePacket(firstSlot, data, size, 0);
        }
        if (data == nullptr) return false;

        const uint64_t fragmentCount =
            (static_cast<uint64_t>(size) + fragmentDataSize - 1) / fragmentDataSize;
        if (fragmentCount > RING_BUFFER_SIZE || size > maxFragmentedMessageSize) return false;

        uint64_t firstSlot = 0;
        if (!reserveSlots(fragmentCount, firstSlot)) return false;

        const uint64_t messageId = nextFragmentMessageId.fetch_add(1, std::memory_order_relaxed);
        uint8_t fragmentData[MAX_PAYLOAD_SIZE];
        uint32_t offset = 0;
        uint64_t fragmentIndex = 0;
        while (offset < size)
        {
            const uint32_t chunkSize =
                (size - offset < fragmentDataSize) ? size - offset : fragmentDataSize;
            const FragmentHeader fragmentHeader{messageId, size, offset};
            std::memcpy(fragmentData, &fragmentHeader, sizeof(fragmentHeader));
            std::memcpy(fragmentData + sizeof(fragmentHeader),
                        static_cast<const uint8_t*>(data) + offset, chunkSize);
            writePacket(firstSlot + fragmentIndex, fragmentData, sizeof(fragmentHeader) + chunkSize,
                        fragmentMarker);
            offset += chunkSize;
            ++fragmentIndex;
        }
        return true;
    }

    bool MemoryQueue::pop(uint32_t& channelId, void* outBuffer, uint32_t bufferSize,
                          uint32_t& outActualSize)
    {
        return popImpl(channelId, outBuffer, bufferSize, outActualSize, nullptr);
    }

    bool MemoryQueue::pop(uint32_t& channelId, DynamicArray<uint8_t>& outBuffer)
    {
        uint32_t actualSize = 0;
        outBuffer.clear();
        return popImpl(channelId, nullptr, 0, actualSize, &outBuffer);
    }

    bool MemoryQueue::popImpl(uint32_t& channelId, void* outBuffer, uint32_t bufferSize,
                              uint32_t& outActualSize,
                              DynamicArray<uint8_t>* dynamicBuffer)
    {
        outActualSize = 0;
        auto resetFragments = [this]() {
            _fragmentBuffer.clear();
            _fragmentMessageId = 0;
            _fragmentTotalSize = 0;
            _fragmentReceived = 0;
            _fragmentChannelId = 0;
        };

        while (true)
        {
            const uint64_t currentWrite = _controlBlock->writeIdx.load(std::memory_order_acquire);
            const uint64_t currentRead = _controlBlock->readIdx.load(std::memory_order_relaxed);
            if (currentRead >= currentWrite) return false;

            const uint64_t slotIdx = currentRead & RING_BUFFER_MASK;
            uint8_t* dataZone = static_cast<uint8_t*>(_basePtr) + sizeof(MemoryControlBlock);
            uint8_t* slotPtr = dataZone + (slotIdx * (sizeof(MessageHeader) + MAX_PAYLOAD_SIZE));
            MessageHeader* header = reinterpret_cast<MessageHeader*>(slotPtr);

            SlotState expected = SlotState::READY;
            if (!header->state.compare_exchange_strong(expected, SlotState::READING,
                                                       std::memory_order_acquire))
            {
                return false;
            }

            const uint32_t packetSize = header->payloadSize;
            const uint32_t marker = header->reserved;
            const uint8_t* packetData = slotPtr + sizeof(MessageHeader);
            auto releaseSlot = [&]() {
                header->reserved = 0;
                header->state.store(SlotState::EMPTY, std::memory_order_release);
                _controlBlock->readIdx.store(currentRead + 1, std::memory_order_release);
            };

            if (marker == fragmentMarker)
            {
                if (packetSize < sizeof(FragmentHeader) || packetSize > MAX_PAYLOAD_SIZE)
                {
                    resetFragments();
                    releaseSlot();
                    continue;
                }

                FragmentHeader fragmentHeader{};
                std::memcpy(&fragmentHeader, packetData, sizeof(fragmentHeader));
                const uint32_t chunkSize = packetSize - sizeof(FragmentHeader);
                const bool validFragment =
                    fragmentHeader.totalSize > MAX_PAYLOAD_SIZE &&
                    fragmentHeader.totalSize <= maxFragmentedMessageSize && chunkSize > 0 &&
                    fragmentHeader.offset <= fragmentHeader.totalSize &&
                    chunkSize <= fragmentHeader.totalSize - fragmentHeader.offset;
                if (!validFragment)
                {
                    resetFragments();
                    releaseSlot();
                    continue;
                }

                if (fragmentHeader.offset == 0)
                {
                    resetFragments();
                    _fragmentBuffer.resize(fragmentHeader.totalSize);
                    _fragmentMessageId = fragmentHeader.messageId;
                    _fragmentTotalSize = fragmentHeader.totalSize;
                    _fragmentChannelId = header->channelId;
                }

                const bool matchesMessage = fragmentHeader.messageId == _fragmentMessageId &&
                                            fragmentHeader.totalSize == _fragmentTotalSize &&
                                            fragmentHeader.offset == _fragmentReceived &&
                                            header->channelId == _fragmentChannelId;
                if (!matchesMessage)
                {
                    resetFragments();
                    releaseSlot();
                    continue;
                }

                std::memcpy(_fragmentBuffer.data() + fragmentHeader.offset,
                            packetData + sizeof(FragmentHeader), chunkSize);
                _fragmentReceived += chunkSize;
                releaseSlot();

                if (_fragmentReceived == _fragmentTotalSize)
                {
                    channelId = _fragmentChannelId;
                    outActualSize = _fragmentTotalSize;
                    const uint32_t copySize =
                        outActualSize < bufferSize ? outActualSize : bufferSize;
                    if (outBuffer && copySize > 0)
                    {
                        std::memcpy(outBuffer, _fragmentBuffer.data(), copySize);
                    }
                    if (dynamicBuffer)
                    {
                        dynamicBuffer->resize(outActualSize);
                        std::memcpy(dynamicBuffer->data(), _fragmentBuffer.data(), outActualSize);
                    }
                    resetFragments();
                    return true;
                }
                continue;
            }

            if (packetSize > MAX_PAYLOAD_SIZE)
            {
                releaseSlot();
                continue;
            }

            channelId = header->channelId;
            outActualSize = packetSize;
            const uint32_t copySize = outActualSize < bufferSize ? outActualSize : bufferSize;
            if (outBuffer && copySize > 0)
            {
                std::memcpy(outBuffer, packetData, copySize);
            }
            if (dynamicBuffer && packetSize > 0)
            {
                dynamicBuffer->resize(packetSize);
                std::memcpy(dynamicBuffer->data(), packetData, packetSize);
            }
            releaseSlot();
            return true;
        }
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