#pragma once
#include "Common/Common.h"
#include "Common/Span.hpp"

class PageCache
{
public:
    static PageCache* GetInstance()
    {
        return &_pageCacheInstance;
    }
    //获取一个k页的span
    Span* NewSpan(size_t k);
    //用brk向系统申请
    static void* myMalloc(size_t size)
    {   
        //当前数据段末尾
        void* old_brk = sbrk(0);
        //要设置的数据段新末尾
        void* new_brk = old_brk + size;
        //用brk函数来调整堆区最高位置
        if (brk(new_brk) == -1) return nullptr;
        //此时，old_brk和new_brk之间的空间就是新申请到的内存空间
        return old_brk;
    }
public:
    std::mutex _mtx;
private:
    SpanList _spanLists[NPAGES];
private:
    //单例模式
    static PageCache _pageCacheInstance;
    PageCache(){}
    PageCache(const PageCache&) = delete;

    //把n页的Span，切分成 n1页的Span 和 n2页的Span，
    //n1页的Span返回，n2页的Span挂载到PageCache的spanList[n2]上
    Span* Split(size_t n, size_t n1, size_t n2);
};