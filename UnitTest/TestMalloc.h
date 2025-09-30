#pragma once
#include "../ConcurrentMalloc.h"
void TestMalloc()
{
    for(int i = 1;i < PAGESIZE;++i)
    {
        void* ptr = ConcurrentMalloc(i);
    }
    void* ptr = ConcurrentMalloc(MAX_SIZES);
}








