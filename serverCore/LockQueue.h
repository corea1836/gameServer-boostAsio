#pragma once

#include "Lock.h"
#include "pch.h"

template <typename T> class LockQueue {
    public:
        void Push(T item) {
            WRITE_LOCK;
            _items.push(item);
        }

        T Pop() {
            WRITE_LOCK;
            if (_items.empty())
                return T();

            T ret = _items.front();
            _items.pop();
            return ret;
        }

        void Pop(vector<T> &items, uint32 count) {
            WRITE_LOCK;
            if (_items.empty())
                return;

            uint32 popCount = 0;
            while (popCount < count && !_items.empty()) {
                items.push_back(Pop());
                popCount++;
            }
        }

        void PopAll(vector<T> &items) {
            WRITE_LOCK;
            while (T item = Pop())
                items.push_back(item);
        }

        void Clear() {
            WRITE_LOCK;
            _items = queue<T>();
        }

        int32 Size() {
            READ_LOCK;
            return _items.size();
        }

    private:
        USE_LOCK;
        queue<T> _items;
};
