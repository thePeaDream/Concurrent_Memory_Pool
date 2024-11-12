#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <pthread.h>
#ifdef __x86_64__
    typedef unsigned long long PAGE_ID; //页数
#elif __i386__
    typedef size_t PAGE_ID;
#endif
using std::cerr;
using std::cin;
using std::cout;
using std::endl; 
static const size_t Max_Bytes = 256 * 1024;
static const size_t FreeList_Number = 208;

// 取对象的头4个字节/头8个字节
void *&nextObj(void *obj)
{
    return *(void **)obj;
}

// 管理 自由链表
class FreeList
{
public:
    // 头插内存块到自由链表里
    void Push(void *obj)
    {
        //*(void**)obj = _freeList;
        nextObj(obj) = _freeList;
        _freeList = obj;
    }
    // 头删自由链表,并把删除的内存块返回
    void *Pop()
    {
        assert(_freeList);
        void *ret = _freeList;
        _freeList = nextObj(_freeList);
        return ret;
    }
    bool Empty()
    {
        return _freeList == nullptr;
    }

private:
    void *_freeList = nullptr;
};

// 计算对象大小的对齐映射规则
class SizeClass
{
public:
    // 整体控制在最多10%左右的内碎片浪费
    //[1,128]   8byte对齐   freeList[0,16)
    //[128+1,1024] 16byte对齐 freeList[16,72)
    //[1024+1,8*1024] 128byte对齐 freeList[72,128)
    //[8*1024+1,64*1024] 1024byte对齐 freeList[128,184)
    //[64*1024+1,256*1024] 8*1024byte对齐 freeList[184,208)
    static size_t _RoundUp(size_t size, size_t alignNum)
    {
        // if(size % alignNum != 0)
        //     size = (size/alignNum+1)*alignNum;
        // return size;
        return ((size + alignNum - 1) & ~(alignNum - 1));
    }
    static size_t RoundUp(size_t size)
    {
        // 对齐以后是多大
        if (size <= 128)
        {
            return _RoundUp(size, 8);
        }
        else if (size <= 1024)
        {
            return _RoundUp(size, 16);
        }
        else if (size <= 8 * 1024)
        {
            return _RoundUp(size, 128);
        }
        else if (size <= 64 * 1024)
        {
            return _RoundUp(size, 1024);
        }
        else if (size <= 156 * 1024)
        {
            return _RoundUp(size, 8 * 1024);
        }
        else
        {
            assert(false);
            return -1;
        }
    }
    // 计算是哪一个桶/自由链表
    static size_t _Index(size_t bytes, size_t align_shift)
    {
        // if(bytes%alignNum == 0)
        // {
        //     return bytes/alignNum - 1;
        // }
        // return bytes/alignNum;
        return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
    }
    static size_t Index(size_t bytes)
    {
        assert(bytes <= Max_Bytes);
        // 每个区间有多少个链
        static int group_array[4] = {16, 56, 56, 56};
        if (bytes <= 128)
        {
            return _Index(bytes, 3);
        }
        else if (bytes <= 1024)
        {
            return _Index(bytes - 128, 4) + group_array[0];
        }
        else if (bytes <= 8 * 1024)
        {
            return _Index(bytes - 1024, 7) + group_array[1] + group_array[0];
        }
        else if (bytes <= 64 * 1024)
        {
            return _Index(bytes - 8 * 1024, 10) + group_array[2] + group_array[1] + group_array[0];
        }
        else if (bytes <= 256 * 1024)
        {
            return _Index(bytes - 64 - 1024, 13) + group_array[3] + group_array[2] + group_array[1] + group_array[0];
        }
        else
            assert(false);
        return -1;
    }
};
// 为什么以8字节对齐最合适，因为内存块挂在自由链表里,64位下一个指针8字节,至少一个内存块要有8字节,自由链表要链接起来,至少一个内存块能存指针大小

// 都以8字节对齐的话，256KB / 8byte,要建很多桶/自由链表   8 16 24 32 40byte………………
