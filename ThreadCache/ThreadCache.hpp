#pragma once
#include "FreeList.hpp"
#include "../Common/AlignMap.hpp"
static const size_t NFREELISTS = BUCKET_NUM;
static const size_t MAX_SIZES = 256 << 10;//256 * 1024 Byte

class ThreadCache;
static thread_local ThreadCache* pTLSThreadCache = nullptr;
//ThreadCache是一个哈希桶结构，每个桶都是一个自由链表
class ThreadCache
{
private:
    FreeList _freeLists[NFREELISTS];
public:
    //提供申请和释放小内存块对象
    void* Allocate(size_t size);
    void Deallocate(void* obj,size_t size);
public:
    //ThreadCache对应的桶，一次向CentralCache申请alignSize大小小内存块对象的数量上限
    static size_t ApplyNumberLimit(size_t alignSize)
    {
        assert(alignSize > 0);
        //小对象，一次批量申请的上限高
        //大对象，一次批量申请的上限低
        //小内存块对象，最大就是MAX_SIZES 256*1024byte
        size_t num = MAX_SIZES / alignSize;
        if(num > 512) num = 512;
        if(num < 2) num = 2;
        return num;
    }
private:
    //向CentralCache申请一批内存块对象
    //index:是哪个自由链表/桶需要申请内存块对象
    //alignSize:申请的内存块对象大小
    void FetchObjectFromCentralCache(size_t index,size_t alignSize);
    void CheckListLength(FreeList& freelist,size_t alignSize);
};


