#pragma once
#include "Common.h"

//管理多个页的 大块内存结构
struct Span
{
    size_t _pageId; //大块内存的起始页号
    size_t _n; //页数

    Span* _next = nullptr;//双向链表
    Span* _prev = nullptr;

    size_t _useCount = 0;//小块内存被分配给thread cache的计数
    void* _freeList = nullptr;//切好的小块内存自由链表

    bool _is_use = false;//判断是否被使用

    size_t _objSize = 0;//这个Span要被切成多个 多大的内存块对象   切好的小对象大小
};

class SpanList
{
public:
    SpanList()
    {
        _head = new Span;
        _head->_next = _head;
        _head->_prev = _head;
    }

    inline bool Empty()
    {
        return _head->_prev == _head;
    }

    inline void Insert(Span* pos,Span* newSpan)
    {
        assert(pos);
        assert(newSpan);

        Span* prev = pos->_prev;
        prev->_next = newSpan;
        newSpan->_prev = prev;
        newSpan->_next = pos;
        pos->_prev = newSpan;
    }
    inline void PushFront(Span* newSpan)
    {
        assert(newSpan);
        Insert(_head->_next,newSpan);
    }

    Span* PopFront()
    {
        assert(!Empty());
        Span* ret = _head->_next;
        Erase(ret);
        return ret;
    }

    inline void Erase(Span* pos)
    {
    	//pos不要删掉，只是解开，然后还要还给page cache
        assert(pos);
        assert(pos != _head);
        Span* prev = pos->_prev;
        Span* next = pos->_next;
        prev->_next = next;
        next->_prev = prev;
        
        pos->_prev = nullptr;
        pos->_next = nullptr;
    }

    Span* begin()
    {
        assert(_head != nullptr);
        assert(_head->_next);
        assert(_head->_next->_next);
        return _head->_next;   
    }

    Span* end()
    {
        assert(_head);
        return _head;
    }
public:
    std::mutex _mtx;
private:
    Span* _head = nullptr;
};