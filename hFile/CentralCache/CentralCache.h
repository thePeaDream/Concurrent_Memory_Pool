#pragma once
#include "Common/Common.h"
#include "Common/Span.hpp"

class CentralCache
{
public:
    //获取唯一的实例
    static CentralCache* GetInstance()
    {
       return &_centralCacheInstance;
    }
    //将一定数量的内存块对象(连续)返回给thread cache
    size_t FetchRangeObj(void*& start, void*& end, size_t n, size_t nsize);
private:
    //获取某个桶中的一个非空的Span
    Span* GetOneSpan(SpanList& list,size_t size);    

    //切分成size大小内存块的span不够，要向Page Cache获取 管理几页 的span
    size_t NumPage(size_t size);
private:
    SpanList _spanLists[208];
private:
    //单例模式实现
    CentralCache(){}
    CentralCache(const CentralCache&) = delete;
    static CentralCache _centralCacheInstance;
};