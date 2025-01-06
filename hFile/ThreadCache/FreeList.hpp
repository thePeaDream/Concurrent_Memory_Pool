#pragma once
#include "Common.h"

static inline void*& NextObj(void* obj)//给一个内存块对象，返回内存块前4/8byte的引用
{
    assert(obj != nullptr);
    return *(void**)obj;
}
//管理切分好的小内存块对象 自由链表
class FreeList
{
public:
    inline void Push(void* obj)//往自由链表里头插内存块
    {
        assert(obj!=nullptr);
        NextObj(obj) = _freeList;
        _freeList = obj;
    }
    inline void PushRange(void* start,void* end)//往自由链表里插入一段连续的内存块
    {
        NextObj(end) = _freeList;
        _freeList = start;
    }
    inline void* Pop()//头删内存块，同时返回第一个内存块
    {
        assert(!Empty());
        void* ret = _freeList;
        _freeList = NextObj(_freeList);
        return ret;
    }
    inline size_t& MaxSize()
    {
        return _maxSize;
    }
    inline bool Empty()
    {
        return _freeList == nullptr;
    }
    
private:
    void* _freeList = nullptr;
    size_t _maxSize = 1;//一次性向CentralCache申请内存块对象的数量
};