#include "PageCache.hpp"
PageCache PageCache::_instance;
Span* PageCache::NewSpan(size_t k)
{
    //从第k页开始往后找到一个空闲的span
    //如果没找到，就要向系统申请空间
    //如果找到了，n(n >= k)页的span要切分成k页的span 和 n-k页的span
    assert(k > 0);
    if(k > NPAGES) // 直接向系统申请
    {
        void* ptr = SystemAlloc(k);
        PAGE_ID pageId = (PAGE_ID)(reinterpret_cast<unsigned long long>(ptr) >> PAGESHIFT);
        Span* span = new Span;
        span->n = k;
        span->pageId = pageId;
        _mapIdToSpan[pageId] = span;
        return span;
    }
    //1 如果在第k个spanList找到span，直接返回
    if(!_spanLists[k].Empty())
    {
        Span* kSpan = _spanLists[k].PopFront();
        //要使用的kSpan，把它包含的页号和Span*的映射关系都要插入_mapIdToSpan
        for(size_t i = kSpan->pageId; i < kSpan->pageId + kSpan->n;++i)
        {
            _mapIdToSpan[i] = kSpan;
        }
        return kSpan;
    }
    //2 从第k+1个位置开始，往后找；只要有一个链表不为空，切分后返回对应k页的span
    for(size_t n = k+1; n <= NPAGES; ++n)
    {
        if(!_spanLists[n].Empty())
        {
            Span* nSpan = _spanLists[n].PopFront();
            //切分成kSpan和otherSpan
            Span* kSpan = new Span;
            Span* otherSpan = new Span;
            
            kSpan->pageId = nSpan->pageId;
            kSpan->n = k;

            otherSpan->pageId = nSpan->pageId + k; 
            otherSpan->n = n - k;
            //将otherSpan添加给PageCache管理，将kSpan返回,还要释放nSpan
            _spanLists[n-k].PushFront(otherSpan);
            //将插入到PageCache里的span，它的最小页和最大页，都插入到_mapIdToSpan中
            _mapIdToSpan[otherSpan->pageId] = otherSpan;
            _mapIdToSpan[otherSpan->pageId + otherSpan->n - 1] = otherSpan;
            delete nSpan;
            //要使用的kSpan，把它包含的页号和Span*的映射关系都要插入_mapIdToSpan
            for(size_t i = kSpan->pageId; i < kSpan->pageId + kSpan->n;++i)
            {
                _mapIdToSpan[i] = kSpan;
            }
            return kSpan;
        }
    }
    //3 没有大页的span了，直接向系统申请NPAGES页的空间，让一个span管理这个空间，插入PageCache
    void* ptr =  SystemAlloc(NPAGES);
    if(ptr == nullptr) throw std::bad_alloc();
    PAGE_ID pageId = ((long long)ptr) >> PAGESHIFT;
    Span* newSpan = new Span;
    newSpan->n = NPAGES;
    newSpan->pageId = pageId;
    _spanLists[NPAGES].PushFront(newSpan);
    //将插入到PageCache里的span，它的最小页和最大页，都插入到_mapIdToSpan中
    _mapIdToSpan[newSpan->pageId] = newSpan;
    _mapIdToSpan[newSpan->pageId + newSpan->n - 1] = newSpan;
    return NewSpan(k);
}

Span* PageCache::ObjectToSpan(void* obj)
{
    PAGE_ID pageId = reinterpret_cast<PAGE_ID>(obj) >> PAGESHIFT;
    unordered_map<PAGE_ID,Span*>::iterator it = _mapIdToSpan.find(pageId);
    if(it == _mapIdToSpan.end())
    {
        assert(false);
        return nullptr;
    }
    return it->second;
}

void PageCache::ReleaseSpanToPageCache(Span* span)
{
    //正在合并的span，也是isUse = true
    assert(span);
    size_t k = span->n;
    span->objSize = 0;
    if(k > NPAGES)
    {
        void* ptr = reinterpret_cast<void*>((span->pageId) << PAGESHIFT);
        SystemFree(ptr,k);
        delete span;
        return;
    }
    //1 插入对应的spanList
    _spanLists[k].PushFront(span);

    //2 对前后的页，尝试进行合并
    while(true)
    {
        size_t prevPage = span->pageId - 1;
        unordered_map<PAGE_ID,Span*>::iterator ret = _mapIdToSpan.find(prevPage);
        //前面页的Span不存在
        if( ret == _mapIdToSpan.end()) break;
        Span* prevSpan = ret->second;
        //前面页的Span不是空闲
        if(prevSpan->isUse == true) break;
        //相邻页合起来的页数 > NPAGES,PageCache无法管理
        if(prevSpan->n + span->n > NPAGES) break;
        //合并相邻页
        span->pageId = prevSpan->pageId;
        span->n += prevSpan->n;
        _spanLists[prevSpan->n].Erase(prevSpan);
        delete prevSpan;
    }
    while(true)
    {
        size_t nextPage = span->pageId + span->n;
        
        auto ret = _mapIdToSpan.find(nextPage);
        if(ret == _mapIdToSpan.end())break;
        Span* nextSpan = ret->second;
        
        if(nextSpan->isUse == true) break;
        
        if(nextSpan->n + span->n > NPAGES) break;

        span->n += nextSpan->n;
        _spanLists[nextSpan->n].Erase(nextSpan);
        delete nextSpan;
    }
    //3 合并完成后，添加span到PageCache对应的spanList中
    _spanLists[span->n].PushFront(span);
    //4 将合并好的span的最大页号和最小页号添加到_mapIdToSpan中
    _mapIdToSpan[span->pageId]= span;
    _mapIdToSpan[span->pageId + span->n - 1] = span;
    span->isUse = false;
}
