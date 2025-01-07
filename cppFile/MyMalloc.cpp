#include "../hFile/MyMalloc.h"
//向Thread Cache申请size byte 空间
void* ConcurrentAlloc(size_t size)
{
    //通过TLS，每个线程无锁获取自己专属的Thread Cache对象
    if(pTLSThreadCache == nullptr)
    {
        pTLSThreadCache = new ThreadCache;
    }
    return pTLSThreadCache->Allocate(size);
}

//把内存块对象还给Thread Cache
void ConcurrentFree(void* ptr,size_t size)
{
    pTLSThreadCache->Deallocate(ptr,size);
}