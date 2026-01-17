#pragma once

#include "Common/Container.hpp"
#include "Common/Exception.hpp"
#include "Common/MacroUtils.hpp"
#include "Common/MemAllocator.hpp"

namespace EgLab
{
    template <class T>
    struct ListNode
    {
        T data;
        ListNode* next;
        ListNode* prev;
    };

    template <class T, class Allocator = StaticSizeAllocator<ListNode<T>>>
    class List : Container<List<T, Allocator>>
    {
        VALUE_ALIAS(T)
    public:
        using IteratorT = Iterator<List<T, Allocator>>;
        using CIteratorT = CIterator<List<T, Allocator>>;
        using ListIterator = Iterator<List<T, Allocator>>;
        using ListCIterator = CIterator<List<T, Allocator>>;
        using Node = ListNode<T>;

        List() : head(nullptr), tail(nullptr)
        {
        }

        List(List&& other)
        {
            head = other.head;
            tail = other.tail;
            other.head = nullptr;
            other.tail = nullptr;
        }

        ~List()
        {
            Node* current = head;
            while (current != nullptr)
            {
                Node* nextNode = current->next;
                allocator.free(current);
                current = nextNode;
            }
        }

        bool empty() const
        {
            return head == nullptr;
        }

        void pushBack(const T& value)
        {
            Node* newNode = static_cast<Node*>(allocator.alloc());
            new (&newNode->data) T(value);
            newNode->next = nullptr;
            newNode->prev = tail;

            if (tail != nullptr)
            {
                tail->next = newNode;
            }
            tail = newNode;

            if (head == nullptr)
            {
                head = newNode;
            }
        }

        void popBack()
        {
            if (tail == nullptr) return;

            Node* nodeToRemove = tail;
            tail = tail->prev;

            if (tail != nullptr)
            {
                tail->next = nullptr;
            }
            else
            {
                head = nullptr;
            }

            nodeToRemove->data.~T();
            allocator.free(nodeToRemove);
        }

        bool erase(Iterator<List<T, Allocator>>& it)
        {
            if (!it.hasNext()) return false;

            Node* nodeToRemove = it._current;
            if (nodeToRemove->prev != nullptr)
            {
                nodeToRemove->prev->next = nodeToRemove->next;
            }
            else
            {
                head = nodeToRemove->next;
            }

            if (nodeToRemove->next != nullptr)
            {
                nodeToRemove->next->prev = nodeToRemove->prev;
            }
            else
            {
                tail = nodeToRemove->prev;
            }

            // nodeToRemove->data.~T();
            allocator.free(nodeToRemove);
            return true;
        }

        Node* pop(IteratorT& it)
        {
            if (!it.hasNext()) return nullptr;
            Node* nodeToPop = it._current;
            it._current = nodeToPop->next;
            if (nodeToPop->prev != nullptr)
            {
                nodeToPop->prev->next = nodeToPop->next;
            }
            else
            {
                head = nodeToPop->next;
            }

            if (nodeToPop->next != nullptr)
            {
                nodeToPop->next->prev = nodeToPop->prev;
            }
            else
            {
                tail = nodeToPop->prev;
            }

            nodeToPop->prev = nullptr;
            nodeToPop->next = nullptr;
            return nodeToPop;
        }

        void pushBack(Node* newNode)
        {
            newNode->next = nullptr;
            newNode->prev = tail;

            if (tail != nullptr)
            {
                tail->next = newNode;
            }
            tail = newNode;

            if (head == nullptr)
            {
                head = newNode;
            }
        }

        Node* getHead() const
        {
            return head;
        }
        Node* getTail() const
        {
            return tail;
        }

        Iterator<List<T, Allocator>> begin() override
        {
            return ListIterator(*this);
        }

        Iterator<List<T, Allocator>> end() override
        {
            auto it = ListIterator(*this);
            it._current = nullptr;
            return it;
        }

        CIterator<List<T, Allocator>> begin() const override
        {
            return ListCIterator(*this);
        }

        CIterator<List<T, Allocator>> end() const override
        {
            auto it = ListCIterator(*this);
            it._current = nullptr;
            return it;
        }

    private:
        Node* head;
        Node* tail;
        static Allocator allocator;
        friend class Iterator<List<T, Allocator>>;
        friend class CIterator<List<T, Allocator>>;
    };

    template <class T, class Allocator>
    class Iterator<List<T, Allocator>>
    {
        VALUE_ALIAS(T)
    public:
        bool hasNext() const
        {
            return _current != nullptr;
        }

        ValueRef next()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException();
            }
            ValueRef data = _current->data;
            _current = _current->next;
            return data;
        }

        ValueCRef operator*() const
        {
            if (!hasNext())
            {
                throw OutOfMemoryException();
            }
            return _current->data;
        }

        ValueRef operator*()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException();
            }
            return _current->data;
        }

        ValueRef data()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No current element in List iterator");
            }
            return _current->data;
        }

        Iterator<List<T, Allocator>> operator++()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException();
            }
            _current = _current->next;
            return *this;
        }

        bool operator==(const Iterator<List<T, Allocator>>& otherIt) const
        {
            return otherIt._current == this->_current;
        }

        Iterator<List<T, Allocator>>& operator=(const Iterator<List<T, Allocator>>& other)
        {
            if (this != &other)
            {
                _list = other._list;
                _current = other._current;
            }
            return *this;
        }

        Iterator<List<T, Allocator>>(const List<T, Allocator>& lst)
            : _list(&lst), _current(lst.head)
        {
        }

        Iterator<List<T, Allocator>>() : _list(nullptr), _current(nullptr)
        {
        }

    private:
        friend class List<T, Allocator>;
        friend class CIterator<List<T, Allocator>>;
        const List<T, Allocator>* _list;
        typename List<T, Allocator>::Node* _current;
    };

    template <class T, class Allocator>
    class CIterator<List<T, Allocator>>
    {
        VALUE_ALIAS(T)
    public:
        bool hasNext() const
        {
            return _current != nullptr;
        }

        ValueCRef next()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException();
            }
            ValueCRef data = _current->data;
            _current = _current->next;
            return data;
        }

        ValueCRef operator*() const
        {
            if (!hasNext())
            {
                throw OutOfMemoryException();
            }
            return _current->data;
        }

        ValueCRef operator*()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException();
            }
            return _current->data;
        }

        ValueCRef data()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException("No current element in List iterator");
            }
            return _current->data;
        }

        CIterator<List<T, Allocator>> operator++()
        {
            if (!hasNext())
            {
                throw OutOfMemoryException();
            }
            _current = _current->next;
            return *this;
        }

        bool operator==(const CIterator<List<T, Allocator>>& otherIt) const
        {
            return otherIt._current == this->_current;
        }

        CIterator<List<T, Allocator>>& operator=(const CIterator<List<T, Allocator>>& other)
        {
            if (this != &other)
            {
                _list = other._list;
                _current = other._current;
            }
            return *this;
        }

        CIterator<List<T, Allocator>>& operator=(CIterator<List<T, Allocator>>&& other)
        {
            if (this != &other)
            {
                _list = other._list;
                _current = other._current;
            }
            return *this;
        }

        CIterator<List<T, Allocator>>& operator=(Iterator<List<T, Allocator>>&& other)
        {
            _list = other._list;
            _current = other._current;
            return *this;
        }

        CIterator<List<T, Allocator>>(const CIterator<List<T, Allocator>>& other)
            : _list(other._list), _current(other._current)
        {
        }

        CIterator<List<T, Allocator>>(const List<T, Allocator>& lst)
            : _list(&lst), _current(lst.head)
        {
        }

        CIterator<List<T, Allocator>>(List<T, Allocator>&& lst) : _list(&lst), _current(lst.head)
        {
        }

        CIterator<List<T, Allocator>>() : _list(nullptr), _current(nullptr)
        {
        }

    private:
        friend class List<T, Allocator>;
        friend class Iterator<List<T, Allocator>>;
        const List<T, Allocator>* _list;
        typename List<T, Allocator>::Node* _current;
    };

    template <class T, class Allocator>
    Allocator List<T, Allocator>::allocator;

} // namespace EgLab