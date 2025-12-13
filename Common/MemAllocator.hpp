#pragma once
/**
 * @file MemAllocator.hpp
 * @author Moussa-Li
 * @brief Memory Allocator
 * @date 2025-10-02
 */

#include <assert.h>

#include <cstddef>
#include <cstring>

#include "Log.hpp"

namespace EgLab
{
    class PushStrategy;
    class PopStrategy;
    class BlockPushStrategy;
    class BlockPopStrategy;
    class ConstExtStrategy;
    class CurrentRecycleStrategy;

    template <class T, class PushStrategyT = PushStrategy, class PopStrategyT = PopStrategy>
    class MemAllocatorBase
    {
    public:
        virtual void* alloc() { return pushStrategy.push_back(sizeof(T)); }

        virtual void* alloc(size_t len) { return pushStrategy.push_back(sizeof(T) * len); }

        virtual void free(void* data) { popStrategy.pop(data, sizeof(T)); }

        MemAllocatorBase()
        {
            pushStrategy.setPopStrategy(&popStrategy);
            popStrategy.setPushStrategy(&pushStrategy);
        }

        ~MemAllocatorBase() {}

    private:
        PushStrategyT pushStrategy;
        PopStrategyT popStrategy;
    };

    template <class T>
    struct MemoryTarit
    {
    };

    template <class T>
    class StaticSizeAllocator : public MemAllocatorBase<T, ConstExtStrategy, CurrentRecycleStrategy>
    {
    };

    /*
    template<class T, class PushStrategyT = PushStrategy, class PopStrategyT = PopStrategy>
    class DynamicSizeAllocator : public MemAllocatorBase<T, PushStrategyT, PopStrategyT>
    {
    public:
        virtual void *alloc() override {
            return nullptr;
        }

        virtual void free(void*) override {

        }

    private:

    };
*/

    struct Block
    {
        char* data;
        size_t tail{0};
        Block* prevBlock{nullptr};
        Block* nextBlock{nullptr};
        size_t blockSize{};
        bool* isValid;
        Block(size_t _blockSize)
        {
            blockSize = _blockSize;
            data = new char[blockSize];
            isValid = new bool[blockSize];
            memset(isValid, false, sizeof(bool) * blockSize);
        }

        ~Block()
        {
            delete[] data;
            delete[] isValid;
        }
    };

    class BlockPrinter
    {
    public:
        BlockPrinter(Block* b) { _b = b; };

        void print()
        {
            int count = 0;
            Block* temp = _b;
            while (temp)
            {
                count++;
                temp = temp->nextBlock;
            }
            int size = count;
            LOG(INFO) << "Block Number : " << size;

            temp = _b;
            count = 0;

            LOG(INFO) << "------------------------";
            while (temp)
            {
                LOG(INFO) << "Block tail : " << temp->tail;
                LOG(INFO) << "Block size : " << temp->blockSize;
                LOG(INFO) << "Block prevBlock : " << temp->prevBlock;
                LOG(INFO) << "Block nextBlock : " << temp->nextBlock;
                count++;
                if (count == size)
                    LOG(INFO) << "------------------------";
                else
                    LOG(INFO) << "-----------↓------------";
                temp = temp->nextBlock;
            }
        }

    private:
        Block* _b;
    };

    class PushStrategy
    {
    public:
        virtual void* push_back(size_t length) = 0;

        void setPopStrategy(PopStrategy* popStrategy) { _popStrategy = popStrategy; }

    protected:
        friend class PopStrategy;

        PopStrategy* _popStrategy;
    };

    class BlockPushStrategy : public PushStrategy
    {
    public:
        virtual void getPos(const void* ptr, Block** b, size_t& pos) = 0;

        virtual ~BlockPushStrategy()
        {
            Block* b = headBlock;
            headBlock = nullptr;
            currentBlock = nullptr;
            while (b)
            {
                Block* delBlock = b;
                b = b->nextBlock;
                delete delBlock;
            }
        }

        void setPopStrategy(BlockPopStrategy* popStrategy)
        {
            _popStrategy = (PopStrategy*)popStrategy;
        }

    protected:
        friend class BlockPopStrategy;

        Block* headBlock{nullptr};

        Block* currentBlock{nullptr};

    private:
        using PushStrategy::setPopStrategy;
    };

    class ConstExtStrategy : public BlockPushStrategy
    {
    public:
        virtual void* push_back(size_t length) override
        {
            if (length > blockSize)
            { // TODO : assert
                return nullptr;
            }
            if (currentBlock == nullptr)
            {
                LOG(INFO) << "new BLock";
                currentBlock = new Block(blockSize);
                headBlock = currentBlock;
                currentBlock->tail += length;
                memset(currentBlock->isValid, true, length * sizeof(bool));
                return (void*)(&currentBlock->data[0]);
            }
            if (currentBlock->tail + length > blockSize)
            {
                LOG(INFO) << "append BLock";
                Block* prevBlock = currentBlock;
                currentBlock = new Block(blockSize);
                currentBlock->prevBlock = prevBlock;
                prevBlock->nextBlock = currentBlock;
                currentBlock->tail += length;
                memset(currentBlock->isValid, true, length * sizeof(bool));
                // LOG(INFO) <<(void*)(&currentBlock->data[0]);
                return (void*)(&currentBlock->data[0]);
            }
            size_t tail = currentBlock->tail;
            currentBlock->tail += length;
            memset(currentBlock->isValid + tail * sizeof(bool), true, length * sizeof(bool));
            return (void*)(&currentBlock->data[tail]);
        }

        virtual void getPos(const void* ptr, Block** b, size_t& pos) override
        {
            if (headBlock == nullptr) return;
            *b = headBlock;
            while (1)
            {
                size_t blockDis =
                    reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>((*b)->data);
                if (blockDis < blockSize)
                {
                    pos = blockDis % blockSize;
                    return;
                }

                *b = (*b)->nextBlock;
            }
        }

    private:
        static constexpr size_t blockSize = 100000;

        friend class CurrentRecycleStrategy;
    };

    class PopStrategy
    {
    public:
        virtual void pop(void* ptr, size_t length) = 0;

        virtual ~PopStrategy() {}

        void setPushStrategy(PushStrategy* pushStrategy) { _pushStrategy = pushStrategy; }

    protected:
        PushStrategy* _pushStrategy;

        friend class PushStrategy;
    };

    class BlockPopStrategy : public PopStrategy
    {
    public:
        void setPushStrategy(BlockPushStrategy* pushStrategy) { _pushStrategy = pushStrategy; }

        virtual ~BlockPopStrategy() {}

    protected:
        inline Block* getHeadBlock() const { return getBlockPushStrategy()->headBlock; };

        inline void getHeadBlock(Block**& b) const { b = &(getBlockPushStrategy()->headBlock); }

        inline Block* getCurrentBlock() const { return getBlockPushStrategy()->currentBlock; }

        inline void getCurrentBlock(Block**& b) const
        {
            b = &(getBlockPushStrategy()->currentBlock);
        }

        BlockPushStrategy* getBlockPushStrategy() const
        {
            if (_pushStrategy == nullptr) return nullptr;
            return static_cast<BlockPushStrategy*>(_pushStrategy);
        }

    private:
        using PopStrategy::setPushStrategy;
    };

    class CurrentRecycleStrategy : public BlockPopStrategy
    {
    public:
        virtual void pop(void* ptr, size_t length)
        {
            assert(_pushStrategy != nullptr);
            size_t pos;
            Block* b;
            ConstExtStrategy* pushStrategy = static_cast<ConstExtStrategy*>(getBlockPushStrategy());
            pushStrategy->getPos(ptr, &b, pos);

            Block** headBlock;
            getHeadBlock(headBlock);

            memset(b->isValid + (pos) * sizeof(bool), false, length * sizeof(bool));
            int idx = b->tail - 1;
            for (; idx >= 0; idx--)
            {
                if (b->isValid[idx])
                {
                    b->tail = idx + 1;
                    break;
                }
            }
            if (idx == -1)
            { // recycle
                Block* next = b->nextBlock;
                Block* prev = b->prevBlock;

                Block** headBlock;
                getHeadBlock(headBlock);
                BlockPrinter printer2(*headBlock);
                printer2.print();
                Block** currentBlock;
                getCurrentBlock(currentBlock);
                if (prev == nullptr)
                {
                    if (next == nullptr)
                    {
                        *headBlock = nullptr;
                        *currentBlock = nullptr;
                        LOG(INFO) << "Delete All Block";
                        delete b;
                        return;
                    }

                    *headBlock = next;
                    (*headBlock)->prevBlock = nullptr;
                    LOG(INFO) << "Delete First Block";
                    delete b;
                    return;
                    // currentBlock
                }

                if (next == nullptr)
                {
                    *currentBlock = b->prevBlock;
                    b->prevBlock->nextBlock = nullptr;
                    LOG(INFO) << "Delete Last Block";
                    delete b;
                    return;
                }

                b->prevBlock->nextBlock = b->nextBlock;
                b->nextBlock->prevBlock = b->prevBlock;
                LOG(INFO) << "Delete Block";
                delete b;
                return;
            }
        }

    private:
    };

} // namespace EgLab