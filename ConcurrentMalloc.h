#pragma once
#include "Common/Common.h"
#include "ThreadCache/ThreadCache.hpp"

static void* ConcurrentMalloc(size_t size)
{
    assert(size > 0);
    assert(size <= MAX_SIZES);
    if(pTLSThreadCache == nullptr)
    {
        pTLSThreadCache = new ThreadCache();
    }
    void* obj = pTLSThreadCache->Allocate(size);
    return obj;
}
static void ConcurrentFree(void* ptr,size_t size)
{

}