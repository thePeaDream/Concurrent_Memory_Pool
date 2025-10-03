#pragma once
#include "Common/Common.h"
#include "Common/AlignMap.hpp"
#include "ThreadCache/ThreadCache.hpp"
#include "PageCache/PageCache.hpp"
#include "Common/SystemAllocFree.h"

static void* ConcurrentMalloc(size_t size)
{
    assert(size > 0);
    size_t alignSize = AlignMap::Align(size);
    if(size <= MAX_SIZES) // <= 256 KB
    {
        //三层缓存
        if(pTLSThreadCache == nullptr)
        {
            static ObjectPool<ThreadCache> _objectPool;
            //如果是同一个对象池，不同线程同时进来，会向同一个对象池申请，需要加锁?
            _objectPool._mtx.lock();
            pTLSThreadCache = _objectPool.New();
            _objectPool._mtx.unlock();
        }
        void* obj = pTLSThreadCache->Allocate(size);
        return obj;
    }
    else // <= NPAGES页 或者 > NPAGES页
    {
        //直接找PageCache申请
        PageCache::GetInstance()->_mtx.lock();
        Span* span = PageCache::GetInstance()->NewSpan(alignSize >> PAGESHIFT);
        span->objSize = alignSize;
        PageCache::GetInstance()->_mtx.unlock();
        void* ptr = reinterpret_cast<void*>(span->pageId << PAGESHIFT);
        return ptr;
    }
}

static void ConcurrentFree(void* ptr)
{
    assert(ptr);
    PageCache::GetInstance()->_mtx.lock();
    Span* span = PageCache::GetInstance()->ObjectToSpan(ptr);
    PageCache::GetInstance()->_mtx.unlock();
    if(span->objSize <= MAX_SIZES) //正常三层缓存释放
    {
        assert(pTLSThreadCache);
        pTLSThreadCache->Deallocate(ptr,span->objSize);
    }
    else //直接还给PageCache
    {
        PageCache::GetInstance()->_mtx.lock();
        PageCache::GetInstance()->ReleaseSpanToPageCache(span);
        PageCache::GetInstance()->_mtx.unlock();
    }
}