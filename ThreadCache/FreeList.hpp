#pragma once
#include "../Common/Common.h"

//用链表组织管理空闲的小内存块对象
class FreeList
{
private:
    //长度
    size_t _size = 0;
    //当前一次向CentralCache申请的小内存块数量
    size_t _allocSize = 1;
    //自由链表起始的小内存块对象
    void* _freeList = nullptr;
public:
    //头插
    void Push(void* obj)
    {
        assert(obj);
        NextObj(obj) = _freeList;
        _freeList = obj;
        ++_size;
    }
    //头删
    void* Pop()
    {
        assert(!Empty());
        void* obj = _freeList;
        _freeList = NextObj(_freeList);
        --_size;
        return obj;
    }
    void PushRange(void* start,void* end,size_t n)
    {
        NextObj(end) = _freeList;
        _freeList = start;
        _size += n;
    }
    void PopRange(void*& start,void*& end,size_t n)
    {
        assert(_size >= n);
        start = end = _freeList;
        //end往后走n-1步
        for(size_t i = 1; i < n ; ++i)
            end = NextObj(end);
        _freeList = NextObj(end);
        NextObj(end) = nullptr;
    }

    size_t& AllocSize()
    {
        return _allocSize;
    }
    size_t Size()
    {
        return _size;
    }
    bool Empty()
    {
        return _size == 0;
    }

};