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
    class List : Container<List<T>>
    {
        VALUE_ALIAS(T)
    public:
        using Node = ListNode<T>;

        List() : head(nullptr), tail(nullptr)
        {
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

        bool erase(Iterator<List<T>>& it)
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

        Node* getHead() const
        {
            return head;
        }
        Node* getTail() const
        {
            return tail;
        }

        Iterator<List<T>>* begin() override
        {
            return new ListIterator(*this);
        }

        Iterator<List<T>>* end() override
        {
            ListIterator* it = new ListIterator(*this);
            it->_current = nullptr;
            return it;
        }

        class ListIterator : public Iterator<List<T>>
        {
        public:
            bool hasNext() const override
            {
                return _current != nullptr;
            }
            ValueRef next() override
            {
                if (!hasNext())
                {
                    throw OutOfMemoryException();
                }
                ValueRef data = _current->data;
                _current = _current->next;
                return data;
            }
            ValueCRef operator*() const override
            {
                if (!hasNext())
                {
                    throw OutOfMemoryException();
                }
                return _current->data;
            }

            ValueRef operator*() override
            {
                if (!hasNext())
                {
                    throw OutOfMemoryException();
                }
                return _current->data;
            }

            ListIterator(const List<T>& lst) : _list(lst), _current(lst.head)
            {
            }

        private:
            friend class List<T>;
            const List<T>& _list;
            typename List<T>::Node* _current;
        };

    private:
        Node* head;
        Node* tail;
        Allocator allocator;
    };

} // namespace EgLab