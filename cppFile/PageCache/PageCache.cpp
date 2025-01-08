#include "../../hFile/PageCache/PageCache.h"
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
    
    //存储n2Span的首尾页的映射关系到哈希表里
    //方便page cache回收内存时，进行合并查找
    _pageShiftSpan[n2Span->_pageId] = n2Span;
    _pageShiftSpan[n2Span->_pageId+n2-1] = n2Span;

    //将要使用的Span和其管理的页号，建立映射，方便Central Cache回收内存时，查找对应Span
    for(PAGE_ID i = 0; i < n1Span->_n; ++i)
    {
        _pageShiftSpan[n1Span->_pageId+i] = n1Span;
    }

    return n1Span;
}
//弹出一个k页的span给 用户 或 CentralCache
Span* PageCache::NewSpan(size_t k)
{
    assert(k > 0 && k < NPAGES);

    //1 先去_spanLists[k]去找span
    if(!_spanLists[k].Empty())
    {
        Span* ret = _spanLists[k].PopFront();
        //将要使用的Span和其管理的页号，建立映射，方便Central Cache回收内存时，查找对应Span
        for(PAGE_ID i = 0; i < ret->_n; ++i)
        {
            _pageShiftSpan[ret->_pageId+i] = ret;
        }
        return ret;
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
    _spanLists[NPAGES-1].PushFront(newSpan);

    //4 切分新申请的span
    return Split(newSpan->_n,k,newSpan->_n-k);
}

//获取内存块对象到Span的映射
Span* PageCache::MemBlockToSpan(void* object)
{
    //先算出页号
    PAGE_ID pageId = (long long)object >> PAGE_SHIFT;
    //在哈希表里找映射的Span*
    std::unordered_map<PAGE_ID,Span*>::iterator ret = _pageShiftSpan.find(pageId);
    if(ret != _pageShiftSpan.end())
    {
        return ret->second;
    }
    return nullptr;
}


void PageCache::ReleaseSpanToPageCache(Span* span)
{
    //对span前后的相邻页，尝试进行合并，缓解内存碎片问题
    
    //往前合并
    while(true)
    {
        PAGE_ID aheadId = span->_pageId - 1;
        auto it = _pageShiftSpan.find(aheadId);
        if(it == _pageShiftSpan.end()) break; //前面的页号不存在
        if(it->second->_is_use == true) break; //前面的span正在被使用
        if(it->second->_n + span->_n >= NPAGES) break; //合并出超过NPAGES页的Span,没办法管理,不能合并
        
        Span* aheadSpan = it->second;
        span->_n += aheadSpan->_n;
        span->_pageId = aheadSpan->_pageId;
        _spanLists[aheadSpan->_n].Erase(aheadSpan);

        //_pageShiftSpan.erase()
        delete aheadSpan;
    }

    //往后合并
    while(true)
    {
        PAGE_ID behindId = span->_pageId + span->_n;
        auto it = _pageShiftSpan.find(behindId);
        if(it == _pageShiftSpan.end()) break; //后面的页号不存在
        if(it->second->_is_use == true) break; //后面的span正在被使用
        if(it->second->_n + span->_n >= NPAGES) break; //合并出超过NPAGES页的Span,没办法管理,不能合并
        
        Span* behindSpan = it->second;
        span->_n += behindSpan->_n;
        _spanLists[behindSpan->_n].Erase(behindSpan);

        //_pageShiftSpan.erase()
        delete behindSpan;
    }

    //挂载到PageCache的桶里
    _spanLists[span->_n].PushFront(span);
    //将首尾页号和span的映射存起来
    _pageShiftSpan[span->_pageId] = span;
    _pageShiftSpan[span->_pageId+span->_n-1] = span;
    span->_is_use = false;
}