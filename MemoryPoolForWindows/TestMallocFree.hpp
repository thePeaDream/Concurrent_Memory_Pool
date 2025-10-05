#pragma once
#include "ConcurrentMalloc.hpp"
static void TestMallocFree()
{
    for (size_t i = 1; i <= ((PageCache::NPAGES) << PAGESHIFT); ++i)
    {
        void* ptr = ConcurrentMalloc(i);
        ConcurrentFree(ptr);
    }
    void* ptr = ConcurrentMalloc(((PageCache::NPAGES) << PAGESHIFT) + 1);
    ConcurrentFree(ptr);
    // for(size_t i = 1; i <= 100; ++i)
    // {
    //     void* ptr = ConcurrentMalloc(i);
    //     ConcurrentFree(ptr);
    // }
    // std::vector<void*> v;
    // for(size_t i = 0; i < 512; ++i)
    // {
    //     v.push_back(ConcurrentMalloc(8));
    // }
    // v.push_back(ConcurrentMalloc(8));

    // for(size_t i = 0; i < 511; ++i)
    // {
    //     ConcurrentFree(v[i]);
    // }
    // ConcurrentFree(v[511]);
    // ConcurrentFree(v[512]);
    // v.clear();
}