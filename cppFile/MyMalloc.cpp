#include "../hFile/MyMalloc.h"
#include "../hFile/PageCache/PageCache.h"
#include "../ObjectPool/ObjectPool.hpp"
//申请size byte 空间
void* ConcurrentAlloc(size_t size)
{
    if(size <= MAX_BYTES)
    {
        //通过TLS，每个线程无锁获取自己专属的Thread Cache对象
        if(pTLSThreadCache == nullptr)
        {
            static ObjectPool<ThreadCache> _objPool;
            pTLSThreadCache = _objPool.New();
        }
        return pTLSThreadCache->Allocate(size);
    }
    else
    {
        //对齐以后的大小(byte)
        size_t alignSize = MapRule::Align(size);
        //页数
        size_t pageNum = (alignSize >> PAGE_SHIFT);
        //直接获取一个pageNum页的span
        PageCache::GetInstance()->_mtx.lock();
        Span* span = PageCache::GetInstance()->NewSpan(pageNum);
        span->_objSize = alignSize;
        PageCache::GetInstance()->_mtx.unlock();
        void* ptr = (void*)((span->_pageId) << PAGE_SHIFT);
        return ptr; 
    }   
}

// //释放内存块
// void ConcurrentFree(void* ptr)
// {
//     Span* span = PageCache::GetInstance()->MemBlockToSpan(ptr);

//     if(span->_objSize > MAX_BYTES)
//     {
//         PageCache::GetInstance()->_mtx.lock();
//         PageCache::GetInstance()->ReleaseSpanToPageCache(span);
//         PageCache::GetInstance()->_mtx.unlock();
//     }
//     else
//     {
//         pTLSThreadCache->Deallocate(ptr,span->_objSize);
//     } 
// }