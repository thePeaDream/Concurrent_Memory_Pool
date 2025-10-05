#pragma once
#include "Common.h"
//以页为单位向系统申请堆空间
inline static void* SystemAlloc(size_t kPage)
{
    void* ptr;
#ifdef _WIN32
    ptr = VirtualAlloc(0, kPage << PAGESHIFT, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (ptr == nullptr)
    {
        throw std::bad_alloc();
    }
#elif __linux__
    ptr = mmap(nullptr, kPage << PAGESHIFT, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED)
    {
        throw std::bad_alloc();
    }
#endif
    return ptr;
}
//直接给系统回收堆空间
inline static void SystemFree(void* ptr, size_t kPage)
{
#ifdef _WIN32
    VirtualFree(ptr, 0, MEM_RELEASE);
#elif __linux__ //linux下用munmap释放堆空间，必须传递大小
    munmap(ptr, kPage << PAGESHIFT);
#endif
}
