#include "PageCache.hpp"
PageCache PageCache::_instance;
Span* PageCache::NewSpan(size_t k)
{
    //从第k页开始往后找到一个空闲的span
    //如果没找到，就要向系统申请空间
    //如果找到了，n(n >= k)页的span要切分成k页的span 和 n-k页的span
    assert(k > 0);
    
    //1 如果在第k个spanList找到span，直接返回
    if(!_spanLists[k].Empty())
    {
        return _spanLists[k].PopFront();
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
            delete nSpan;
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
    return NewSpan(k);
}