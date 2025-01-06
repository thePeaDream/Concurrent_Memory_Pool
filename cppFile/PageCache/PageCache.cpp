#include "../hFile/PageCache/PageCache.h"
PageCache PageCache::_pageCacheInstance;

//把n页的Span，切分成 n1页的Span 和 n2页的Span
//n1页的Span返回，n2页的Span挂载到PageCache的spanList[n2]上
Span* PageCache::Split(size_t n, size_t n1, size_t n2)
{
    assert(n == n1+n2);
    //1 nSpan要弹出来，被切分
    Span* nSpan = _spanLists[n].PopFront();
    Span* n1Span = new Span;
    Span* n2Span = new Span;

    //2 用页号切分
    //n1Span的起始页号为
    //nSpan->_pageId;
    n1Span->_pageId = nSpan->_pageId;
    n1Span->_n = n1;
    //n2Span的起始页号为
    //nSpan->_pageId + n1;
    n2Span->_pageId = nSpan->_pageId + n1;
    n2Span->_n = n2;

    //3 n2页的Span挂载到PageCache的spanList[n2]上
    _spanLists[n2].PushFront(n2Span);

    return n1Span;
}
//获取一个k页的span
Span* PageCache::NewSpan(size_t k)
{
    assert(k > 0 && k < NPAGES);
    //1 先去_spanLists[k]去找span
    if(!_spanLists[k].Empty())
    {
        return _spanLists[k].PopFront();
    }
    
    //2 若没有，往后面的桶里找span(n页)，把它切分成 k页 的 span
    for(size_t n = k+1; n < NPAGES; ++n)
    {
        if(!_spanLists[n].Empty())
        {
            //切分成 k页  和  (n-k)页 的span
            //k页的span返回给central cache，(n-k)页的span挂载到第(n-k)号桶
            return Split(n,k,n-k);
        }
    }
    //3 若Page Cache没有合适的span了，就向系统申请一块NPAGE - 1页的Span
    void* memory = myMalloc((NPAGES - 1) << 13);
    if(memory == nullptr)
        throw std::bad_alloc();
    Span* newSpan = new Span;
    newSpan->_pageId = (long long)memory >> 13;
    newSpan->_n = NPAGES-1;

    //4 切分新申请的span
    return Split(newSpan->_n,k,newSpan->_n-k);
}