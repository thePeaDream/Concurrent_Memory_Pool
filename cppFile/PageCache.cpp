#include "../hFile/PageCache.h"

PageCache PageCache::_pageCacheInstance;

//弹出一个k页的span给 用户 / CentralCache
Span* PageCache::NewSpan(size_t k)
{
	assert(k > 0);
	if (k >= NPAGES)
	{
		//直接找堆要空间
		void* ptr = SystemAlloc(k);
		//Span* ret = new Span;
		Span* ret = _spanPool.New();
		ret->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
		ret->_n = k;
		//_idSpanMap[ret->_pageId] = ret;
		_idSpanMap.set(ret->_pageId, ret);
		return ret;
	}
	//先检查当前页是否有span
	if (!_spanLists[k].Empty())
	{
		Span* ret = _spanLists[k].PopFront();
		//建立id和span*的映射，方便CentralCache回收小块内存时，查找对应的Span
		for (PAGE_ID i = 0; i < ret->_n; ++i)
		{
			_idSpanMap.set(ret->_pageId + i, ret);
		}
		return ret;
	}
	// 检查后面的桶里面有没有span，如果有可以把它切分
	for (size_t i = k + 1; i < NPAGES; ++i)
	{
		if (!_spanLists[i].Empty())
		{
			//开始切分 k n-k
			//k页的span返回给central cache n-k页的span挂到第n-k个桶里
			Span* kSpan = _spanPool.New();
			Span* nSpan = _spanLists[i].PopFront();

			//在nSpan的头部切一个k页下来
			kSpan->_n = k;
			kSpan->_pageId = nSpan->_pageId;

			nSpan->_pageId += k;
			nSpan->_n -= k;

			_spanLists[nSpan->_n].PushFront(nSpan);
			//存储nSpan的起始和结束页号与nSpan*的映射，方便合并
			_idSpanMap.set(nSpan->_pageId, nSpan);
			_idSpanMap.set(nSpan->_pageId + nSpan->_n - 1,nSpan);
			//建立id和span*的映射，方便CentralCache回收小块内存时，查找对应的Span
			for (PAGE_ID i = 0; i < kSpan->_n; ++i)
			{
				_idSpanMap.set(kSpan->_pageId+i, kSpan);
			}
			return kSpan;
		}
	}
	//走到这里说明后面没有大页的span了，向系统申请128页的span
	Span* bigSpan = _spanPool.New();
	void* ptr = SystemAlloc(NPAGES-1);


	bigSpan->_pageId = ((PAGE_ID)ptr >> PAGE_SHIFT);

	bigSpan->_n = NPAGES - 1;

	_spanLists[NPAGES - 1].PushFront(bigSpan);
	return NewSpan(k);
}
//获取内存块对象到Span的映射
Span* PageCache::MemBlockToSpan(void* object)
{
    //先算出页号
    PAGE_ID pageId = (PAGE_ID)object >> PAGE_SHIFT;
    //把锁对象给了RAII类，出了作用域自动解锁
    //std::unique_lock<std::mutex> lock(_mtx);

    //在哈希表里找映射的Span*
    Span* ret = (Span*)_idSpanMap.get(pageId);
    if(ret != nullptr)
		return ret;
	assert(false);
	return nullptr;
}

void PageCache::ReleaseSpanToPageCache(Span* span)
{
    //大于128页直接还给堆
    if(span->_n >= NPAGES)
    {
        //说明是直接找堆要的空间
        void* ptr = (void*)((span->_pageId) << PAGE_SHIFT);
        SystemFree(ptr,span->_n);
        _spanPool.Delete(span);
        return;
    }
    //对span前后的相邻页，尝试进行合并，缓解内存碎片问题
    //往前合并
    while(true)
    {
        PAGE_ID aheadId = span->_pageId - 1;
        Span* aheadSpan = (Span*)_idSpanMap.get(aheadId);
        if(aheadSpan == nullptr) break; //前面的页号不存在
        if(aheadSpan->_isUse == true) break; //前面的span正在被使用
        if(aheadSpan->_n + span->_n >= NPAGES) break; //合并出超过NPAGES页的Span,没办法管理,不能合并
        
        span->_n += aheadSpan->_n;
        span->_pageId = aheadSpan->_pageId;
        _spanLists[aheadSpan->_n].Erase(aheadSpan);

        _spanPool.Delete(aheadSpan);
    }

    //往后合并
    while(true)
    {
        PAGE_ID behindId = span->_pageId + span->_n;
        Span* behindSpan = (Span*)_idSpanMap.get(behindId);
        if(behindSpan == nullptr) break; //后面的页号不存在
        if(behindSpan->_isUse == true) break; //后面的span正在被使用
        if(behindSpan->_n + span->_n >= NPAGES) break; //合并出超过NPAGES页的Span,没办法管理,不能合并
        
        span->_n += behindSpan->_n;
        _spanLists[behindSpan->_n].Erase(behindSpan);
        _spanPool.Delete(behindSpan);
    }

    //挂载到PageCache的桶里
    _spanLists[span->_n].PushFront(span);
    //将首尾页号和span的映射存起来
    _idSpanMap.set(span->_pageId , span);
    _idSpanMap.set(span->_pageId+span->_n-1, span);
    span->_isUse = false;
}