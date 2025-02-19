#pragma once
//公共头文件
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <atomic>

#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#ifdef __linux__
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#elif define(_WIN32)
#include <windows.h>
#endif


using std::cout;
using std::cin;
using std::cerr;
using std::endl;

//一页 = 2^13 = 8K     如果是4K改成12
static const size_t PAGE_SHIFT = 13;
//申请<= MAX_BYTES的内存块，都直接向Thread Cache申请
static const size_t MAX_BYTES = 256*1024;
//Page Cache有(NPAGES - 1)个桶，Span最多管理(NPAGES - 1)页
static const size_t NPAGES = 129;
//Thread Cache和Central Cache里 小内存块大小 和 桶的映射关系 决定了有NFREELIST个桶 
static const size_t NFREELIST = 208;

typedef size_t PAGE_ID;

static inline void*& NextObj(void* obj)//给一个内存块对象，返回内存块前4/8byte的引用
{
    return *(void**)obj;
}

//按页为单位，向系统申请
static void* SystemAlloc(size_t page)
{
    void* ret = nullptr;
#ifdef __linux__   
    //用mmap向系统申请
    ret = mmap(nullptr,page << PAGE_SHIFT,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
    if(ret == MAP_FAILED) return nullptr;
#elif define(_WIN32)
    ret = VirtualAlloc(0, page << PAGE_SHIFT, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#endif
    return ret;
}

static void SystemFree(void* ptr,size_t page)
{
#ifdef __linux__
    munmap(ptr,page << PAGE_SHIFT);
#elif define(_WIN32)
    VirtualFree(ptr, 0, MEM_RELEASE);
#endif
}
