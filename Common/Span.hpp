#pragma once
#include "Common.h"
//管理多个页的大块内存
struct Span
{
    //页号
    PAGE_ID pageId = 0;
    //页数
    size_t n = 0;
    //切分成的小内存块链表
    void* freeList = nullptr;
    //用链表结构组织起来
    Span* next = nullptr;
    Span* prev = nullptr;
    //切分出来的小块内存的大小
    size_t objSize = 0;

    //小内存块的使用计数
    size_t useCount = 0;
    //是否正在被使用（是否在PageCache里空闲）
    bool isUse = false;
};

struct spanIterator
{
    Span* _cur;
    explicit spanIterator(Span* cur)
    :_cur(cur)
    {}
    //前置++
    spanIterator& operator++()
    {
        _cur = _cur->next;
        return *this;
    }
    //后置++
    spanIterator operator++(int)
    {
        spanIterator ret(_cur);
        _cur = _cur->next;
        return ret;
    }
    //前置--
    spanIterator& operator--()
    {
        _cur = _cur->prev;
        return *this;
    }
    //后置--
    spanIterator operator--(int)
    {
        spanIterator ret(_cur);
        _cur = _cur->prev;
        return ret;
    }
    //!= ==
    bool operator!=(const spanIterator& it)const 
    {
        if(_cur != it._cur) return true;
        return false;
    }
    bool operator==(const spanIterator& it)const
    {
        return !(*this != it);
    }
    Span& operator*()const
    {
        return *_cur;
    }
    Span* operator->()const
    {
        return _cur;
    }
    //隐式转换
    operator Span*()const
    {
        return _cur;
    }
};
//带头双向循环链表
class SpanList
{
private:
    Span* _head = nullptr;
    size_t _size = 0;
public:
    std::mutex _mtx;
public:
    typedef spanIterator iterator; 
    SpanList()
    {
        _head = new Span;
        _head->next = _head->prev = _head;
    }
    bool Empty()
    {
        return _size == 0;
    }
    iterator Begin()
    {
        return iterator(_head->next);
    }
    iterator End()
    {
        return iterator(_head);
    }

    void Erase(Span* pos)
    {
        assert(pos);
        assert(pos != _head);
        Span* test = _head;
        size_t i = 0;
        while(test->next != _head)
        {
            i++;
            test = test->next;
        }
        assert(i == _size);
        Span* prev = pos->prev;
        Span* next = pos->next;
        prev->next = next;
        next->prev = prev;
        pos->next = nullptr;
        pos->prev = nullptr;
        --_size;
    }

    Span* PopFront()
    {
        //头删，返回从链表中被删除的Span*
       assert(_head != _head->next);
       Span* del = _head->next;
       Erase(del);
       return del;
    }
    void PushFront(Span* newSpan)
    {
        assert(newSpan);
        Span* next = _head->next;
        _head->next = newSpan;
        newSpan->prev = _head;
        newSpan->next = next;
        next->prev = newSpan;
        ++_size;
    }
};