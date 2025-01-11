#pragma once
#include "../Common/Common.h"
#include "FreeList.hpp"
#include "../Common/MapRule.h"
class ThreadCache
{
public:
    //向ThreadCache申请和释放内存块对象
    void* Allocate(size_t size);
    void Deallocate(void* ptr,size_t size);

    //thread cache向中心缓存一次批量申请 内存块对象 的数量上限
    static size_t GetMemBlockNumberTopLimit(size_t size)
    {
        if(size == 0) return 0;
        //[2,512],一次申请多少个对象
        int num = MAX_BYTES / size;
        //大对象，一次批量申请的上限低
        if(num < 2)
            num = 2;
        //小对象，一次批量申请的上限高
        else if(num > 512)
            num = 512;
        return num;
    } 
private:
    //index位置自由链表的内存块不够时，向Central Cache获取alignSize大小的内存块对象，内部会根据情况获取数量
    void* FetchFromCentralCache(size_t index,size_t alignSize);
    
    void FreeMemblockToCentralCache(FreeList& freeList,size_t size);
private:
    FreeList _freeLists[208];
};

//线程本地存储，每个线程创建时都私有一份
static thread_local ThreadCache* pTLSThreadCache = nullptr;