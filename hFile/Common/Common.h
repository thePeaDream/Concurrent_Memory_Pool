#pragma once
//公共头文件
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include <atomic>

#include <sys/mman.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
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

typedef long long PAGE_ID;

static inline void*& NextObj(void* obj)//给一个内存块对象，返回内存块前4/8byte的引用
{
    //条件断点
    // if(obj == nullptr)
    // {
    //     int a = 1;
    // }
    return *(void**)obj;
}