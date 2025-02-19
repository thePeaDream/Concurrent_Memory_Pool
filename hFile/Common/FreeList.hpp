#pragma once
#include "Common.h"


//管理切分好的小内存块对象 自由链表
class FreeList
{
public:
    inline void Push(void* obj)//往自由链表里头插内存块
    {
        assert(obj!=nullptr);
        
        NextObj(obj) = _freeList;
        _freeList = obj;
        _size++;
    }
    inline void PushRange(void* start,void* end,size_t num)//往自由链表里头插一段连续的内存块对象
    {
        assert(start);
        assert(end);

        NextObj(end) = _freeList;
        _freeList = start;
        _size += num;
        
    }
    void PopRange(void*& start, void*& end, size_t num)//头删num个内存块对象，外部用start、end取得这些内存块对象 
    {
        assert(num <= _size);
        start = end =  _freeList;
        for(size_t i = 0 ; i < num - 1; ++i)
        {
            end = NextObj(end);
        }

        _freeList = NextObj(end);
        NextObj(end) = nullptr;
        _size = _size - num;
    }
    inline void* Pop()//头删内存块，同时返回第一个内存块
    {
        assert(!Empty());
        void* ret = _freeList;

        _freeList = NextObj(_freeList);
        _size--;
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
    size_t Size()
    {
        return _size;
    }
private:
    void* _freeList = nullptr;
    size_t _maxSize = 1;//当前桶一次性向CentralCache申请内存块对象的数量
    size_t _size = 0;//当前桶的内存块个数
};