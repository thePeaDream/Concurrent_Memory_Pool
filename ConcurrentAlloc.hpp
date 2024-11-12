#pragma once
#include "Common.h"
#include "ThreadCache.hpp"


static void* ConcurrentAlloc(size_t size)
{
    //通过TLS，每个线程无锁的获取自己专属的ThreadCache对象
    if(pTLSThreadCache == nullptr)
    {
        pTLSThreadCache = new ThreadCache;
    }
    return pTLSThreadCache->Allocate(size); 
}

static void* ConcurrentFree(void* ptr,size_t size)
{
    assert(pTLSThreadCache);

    pTLSThreadCache->Deallocate(ptr,size);
}
