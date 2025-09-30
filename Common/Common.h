#pragma once
#include <cassert>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <thread>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>


//不同平台系统申请空间的头文件
#ifdef _WIN32
    #include <windows.h>
#elif __linux__
    #include <unistd.h>
    #include <sys/mman.h>
#endif

//不同平台定义PAGE_ID
#ifdef _WIN64
    typedef unsigned long long PAGE_ID;
#elif _WIN32
    typedef size_t PAGE_ID;
#elif __x86_64__//linux下64位的宏
    typedef unsigned long long PAGE_ID;
#elif __i386__//linux下32位的宏
    typedef size_t PAGE_ID;
#endif

//一页的大小
static const size_t PAGESIZE = 4096;
static const size_t PAGESHIFT = 12;

using std::cout;
using std::cin;
using std::endl;
using std::vector;
using std::map;
using std::unordered_map;

static void*& NextObj(void* obj)//取头4Byte或8Byte的空间
{
    return *(void**)obj;
}