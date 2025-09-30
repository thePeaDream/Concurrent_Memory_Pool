#include "CentralCache.hpp"
#include "../PageCache/PageCache.hpp"
CentralCache CentralCache::_instance;

size_t CentralCache::FetchRangeObject(void*& start,void*& end,size_t n,size_t alignSize)
{
    size_t index = AlignMap::Mapping(alignSize);
    //加桶锁
    _spanLists[index]._mtx.lock();
    //从SpanList中，找到一个非空的Span
    Span* span = GetOneSpan(_spanLists[index],alignSize);
    assert(span);
    assert(span->freeList);
    //在这个span里，切出n个alignSize的小内存块对象，放到start end中
    start = end = span->freeList;
    size_t ret = 1;
    //注意一个span可能没有这么多的小内存块对象了
    for(size_t i = 0; i < n - 1; ++i)
    {
        if(NextObj(end) == nullptr) 
            break;
        ++ret;
        end = NextObj(end);
    }
    span->freeList = NextObj(end);
    NextObj(end) = nullptr;
    _spanLists[index]._mtx.unlock();
    return ret;
}

Span* CentralCache::GetOneSpan(SpanList& spanList,size_t alignSize)
{
    //遍历整个spanList
    //如果spanList为空，或者所有的span都没有空间了span->freeList == nullptr，就要向PageCache申请一个k页的span，否则直接返回非空的 span 对象
    SpanList::iterator it = spanList.Begin();
    while(it != spanList.End())
    {
        if(it->freeList)
            return it;
        else
            ++it;
    }
    //走到这里，说明所有的span都没有空间了 或者 spanList为空
    //向PageCache申请一个k页的span
    size_t k = ApplyKSpan(alignSize);
    //先解开桶锁
    //当要把申请的span挂到SpanList时，再重新加上桶锁
    spanList._mtx.unlock();
    PageCache::GetInstance()->_mtx.lock();
    Span* newSpan = PageCache::GetInstance()->NewSpan(k);
    newSpan->isUse = true;
    newSpan->objSize = alignSize;
    PageCache::GetInstance()->_mtx.unlock();
    
    //将newSpan管理的大页空间，切分成多个小内存块对象，用链表组织起来
    SplitSpan(newSpan,alignSize);

    spanList._mtx.lock();
    spanList.PushFront(newSpan);
    return newSpan;
}

void CentralCache::SplitSpan(Span* span,size_t objSize)
{
    //1 通过页号计算起始地址
    char* start = reinterpret_cast<char*>((span->pageId) << PAGESHIFT);
    //2 通过页数和起始地址计算结束地址
    size_t sumSize = (span->n) << PAGESHIFT;
    char* end = start + sumSize;
    //3 把大块内存切成自由链表
    //不断尾插 “没有被切分的大块内存部分”
    span->freeList = start; 
    void* tail = span->freeList;
    char* part = start + objSize;
    while(part < end)
    {
        NextObj(tail) = part;
        tail = NextObj(tail);
        part += objSize;
    }
    NextObj(tail) = nullptr;
}
