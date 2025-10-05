#pragma once
#include "ConcurrentMalloc.hpp"
#include "Common.h"
static void TestMutiThreadAlloc()
{
    std::vector<void*> v;
    for (int i = 0; i < 5; ++i)
    {
        void* ptr = ConcurrentMalloc(5);
        v.push_back(ptr);
    }
    for (auto e : v)
    {
        ConcurrentFree(e);
    }
}

static void TestMutiThread()
{
    std::thread t1(TestMutiThreadAlloc);
    std::thread t2(TestMutiThreadAlloc);
    t1.join();
    t2.join();
}

