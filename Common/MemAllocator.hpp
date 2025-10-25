#pragma once
/**
 * @file MemAllocator.hpp
 * @author Moussa-Li
 * @brief Memory Allocator
 * @date 2025-10-02
 */

#include <cstddef>
 #include <string.h>
 #include <assert.h>

namespace EgLab
{
    class PushStrategy;
    class PopStrategy;
    class ConstExtStrategy;
    class CurrentRecycleStrategy;

    template<class T, class PushStrategyT = PushStrategy, class PopStrategyT = PopStrategy>
    class MemAllocatorBase
    {
    public:
        virtual void *alloc() {
            return pushStrategy.push_back(sizeof(T));
        }

        virtual void free(void* data) {
            popStrategy.pop(data, sizeof(T));
        }

        MemAllocatorBase()
        {
            pushStrategy.setPopStrategy(&popStrategy);
            popStrategy.setPushStrategy(&pushStrategy);
        }

    private:
        PushStrategyT pushStrategy;
        PopStrategyT popStrategy;
    };

    template<class T>
    struct MemoryTarit
    {

    };

    template<class T>
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
        char *data;
        size_t tail{0};
        Block* prevBlock{nullptr};
        Block* nextBlock{nullptr};
        size_t blockSize{};
        bool *isValid;
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

    class PushStrategy
    {
    public:
        virtual void *push_back(size_t length) = 0;

        virtual void getPos(const void* ptr, Block* b, size_t &pos) = 0;
        

        void setPopStrategy(PopStrategy* popStrategy)
        {
            _popStrategy = popStrategy;
        }

    protected:

        friend class PopStrategy;
        friend class CurrentRecycleStrategy;

        Block *headBlock{nullptr};

        Block *currentBlock{nullptr};

        PopStrategy *_popStrategy;

    };

    

    class ConstExtStrategy : public PushStrategy
    {
    public:
        virtual void *push_back(size_t length)override {
            if(length > blockSize)
            { // TODO : assert
                return nullptr;
            }
            if(currentBlock == nullptr)
            {
                currentBlock = new Block(blockSize);
                headBlock = currentBlock;
                currentBlock->tail += length;
                memset(currentBlock->isValid, true, length * sizeof(bool));
                return (void*)(&currentBlock->data[0]);
            }
            if(currentBlock->tail + length > blockSize)
            {
                Block *prevBlock = currentBlock;
                currentBlock = new Block(blockSize);
                currentBlock->prevBlock = prevBlock;
                prevBlock->nextBlock = currentBlock;
                currentBlock->tail += length;
                memset(currentBlock->isValid, true, length * sizeof(bool));
                return (void*)(&currentBlock->data[0]);
            }
            size_t tail = currentBlock->tail;
            currentBlock->tail += length;
            memset(currentBlock->isValid + tail*sizeof(bool), true, length * sizeof(bool));
            return (void*)(&currentBlock->data[tail]);
        }

        virtual void getPos(const void* ptr, Block* b, size_t &pos) override {
            if(headBlock == nullptr)
                return;
            size_t blockDis = reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(headBlock->data);
            size_t blockNum = blockDis / blockSize;
            pos = blockDis % blockSize;
            b = headBlock;
            for(int i = 0 ; i < blockNum; i++)
            {
                b = b->nextBlock;
            }

        } 

    private:
        static constexpr size_t blockSize = 1000000;

        


    };

    class PopStrategy
    {
    public:
        virtual void pop(void* ptr, size_t length) = 0;


        void setPushStrategy(PushStrategy* pushStrategy)
        {
            _pushStrategy = pushStrategy;
        }

    protected:
        PushStrategy *_pushStrategy;

        friend class PushStrategy;

    };

    class CurrentRecycleStrategy : public PopStrategy
    {
    public:
        virtual void pop(void* ptr, size_t length) {
            assert(_pushStrategy != nullptr);
            size_t pos;
            Block* b;
            _pushStrategy->getPos(ptr, b, pos);

            memset(b->isValid + pos*sizeof(bool), false, length * sizeof(bool));
            size_t idx = b->tail - 1;
            for(; idx >= 0; idx--)
            {
                if(b->isValid[idx])
                    break;
            }
            if(idx == 0)
            { //recycle
                Block* next = b->nextBlock;
                Block* prev = b->prevBlock;
                if(prev == nullptr)
                {
                    _pushStrategy->headBlock = nullptr;
                    _pushStrategy->currentBlock = nullptr;
                    delete b;
                    return;
                }

                if(next == nullptr)
                {
                    _pushStrategy->currentBlock = b->prevBlock;
                    b->prevBlock->nextBlock = nullptr;
                    delete b;
                    return;
                }

                b->prevBlock->nextBlock = b->nextBlock;
                b->nextBlock->prevBlock = b->prevBlock;
                delete b;
                return;
            }
        }

    private:

    };
    
}