#pragma once
#include "../ConcurrentMalloc.h"
void TestMallocFree()
{
    for(size_t i = 1; i < PageCache::GetInstance()->NPAGES << PAGESHIFT; i++)
    {
        void* ptr = ConcurrentMalloc(i);
        ConcurrentFree(ptr);
    }
}