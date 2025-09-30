#pragma once
#include "../Common/Common.h"
#include "../Common/AlignMap.hpp"
#include "../Common/Span.hpp"
#include "../ThreadCache/ThreadCache.hpp"

class CentralCache
{
private:
    static const size_t NSPANLISTS = BUCKET_NUM;
    SpanList _spanLists[NSPANLISTS];
//单例模式
private:
    CentralCache(){}
    CentralCache(const CentralCache&) = delete;
    CentralCache& operator=(const CentralCache&) = delete;
    static CentralCache _instance;
public:
    static CentralCache* GetInstance()
    {
        return &_instance;
    }
public:
    size_t FetchRangeObject(void*& start,void*& end,size_t n,size_t alignSize);
private:
    //获取一个非空的span
    Span* GetOneSpan(SpanList& index,size_t alignSize);
    //要申请几页的span
    static size_t ApplyKSpan(size_t alignSize)
    {
        //根据要切分的内存块大小，计算向PageCache获取几页的Span

        //对于特定内存块大小,ThreadCache一次向CentralCache申请的最大空间(byte) = ThreadCache一次向CentralCache申请的alignSize内存块数量上限 * 一个内存块大小
        size_t limit = ThreadCache::ApplyNumberLimit(alignSize) * alignSize;
        size_t pageNum = limit >> PAGESHIFT;
        if(pageNum == 0) 
            pageNum = 1;
        return pageNum;
    }
    //切分Span的大页空间成多个小内存块对象，用链表组织起来
    void SplitSpan(Span* span,size_t objSize);
};


