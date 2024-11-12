#pragma once
#include "Common.h"

class ThreadCache;

//每个线程在被创建时有自己独立的pTLSThreadCache
static __thread ThreadCache* pTLSThreadCache = nullptr;

class ThreadCache{
public:
    //申请和释放内存对象
    void* Allocate(size_t size);
    void Deallocate(void* ptr,size_t size);//给了size才能算这个内存块应该放在哪个桶里/哪个自由链表里
    void* FetchFromCentralCache(size_t index,size_t size);//从中央缓存获取对象
private:
    FreeList _freeLists[FreeList_Number];
};

void* ThreadCache::FetchFromCentralCache(size_t index,size_t size)
{

    return nullptr;
}
 
void* ThreadCache::Allocate(size_t size)
{
    assert(size <= Max_Bytes);
    //对齐以后的申请大小
    size_t alignSize = SizeClass::RoundUp(size);
    //对应哪一个桶/自由链表
    //整体控制在最多10%左右的内碎片浪费
    //[1,128]   8byte对齐   freeList[0,16)
    //[128+1,1024] 16byte对齐 freeList[16,72)
    //[1024+1,8*1024] 128byte对齐 freeList[72,128)
    //[8*1024+1,64*1024] 1024byte对齐 freeList[128,184)
    //[64*1024+1,256*1024] 8*1024byte对齐 freeList[184,208)
    size_t index = SizeClass::Index(alignSize);
    if(!_freeLists[index].Empty())
    {
        return _freeLists[index].Pop();
    }
    else
    {
        //这个自由链表下面没有内存块
        //从中心缓存去获取内存块对象
    }

}

void ThreadCache::Deallocate(void* ptr,size_t size)
{
    assert(size <= Max_Bytes);
    assert(ptr);
    //放到第几个自由链表
    size_t index = SizeClass::Index(size);
    _freeLists[index].Push(ptr);
}