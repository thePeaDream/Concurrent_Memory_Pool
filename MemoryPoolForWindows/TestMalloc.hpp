#pragma once
#include "ConcurrentMalloc.hpp"
static void TestMalloc()
{
    for (int i = 1;i < PAGESIZE;++i)
    {
        void* ptr = ConcurrentMalloc(i);
    }
    void* ptr = ConcurrentMalloc(MAX_SIZES);
}