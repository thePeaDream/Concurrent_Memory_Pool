#pragma once
#include "Common.h"
//管理以页为单位的大块内存
struct Span
{
    PAGE_ID _pageId = 0;//大块内存的起始页号
    size_t _n = 0;//页的数目
    
    Span* _next = nullptr;
    Span* _prev = nullptr;
    
    void* _freeList = nullptr;//切好的小块内存的自由链表
    size_t _usecount = 0;//使用计数，==0说明所有对象都回来了,切好的小块内存，被分配给thread cache的计数
};

//带头双向循环链表
class SpanList
{
public:
    SpanList(){
        _head = new Span;
        _head->_next = _head;
        _head->_prev = _head;
    }
    void insert(Span* pos,Span* newSpan)
    {
        assert(pos);
        assert(newSpan);
        Span* prev = pos->_prev;
        prev->_next=newSpan;
        newSpan->_prev=prev;
        newSpan->_next=pos;
        pos->_prev=newSpan;
    }
    void Erase(Span* pos)
    {
        assert(pos);
        assert(pos != _head);
        Span* prev = pos->_prev;
        Span* next = pos->_next;
        prev->_next = next;
        next->_prev = prev;
    }
private:
    Span* _head = nullptr;
};
class CentralCache
{
private:
    SpanList _spanLists[FreeList_Number];
public:
     

};
