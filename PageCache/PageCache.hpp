#ifndef __PAGECACHE__
#define __PAGECACHE__
#include "../Common/AlignMap.hpp"
#include "../Common/Span.hpp"
#include "../Common/Common.h"
#include "../Common/SystemAllocFree.h"

class PageCache
{
private:
    //第0个占位
    static const size_t NPAGES = 128; 
    SpanList _spanLists[NPAGES + 1];
public:
    std::mutex _mtx;
//单例模式
private:
    PageCache(){}
    PageCache(const PageCache&) = delete;
    PageCache& operator=(const PageCache&) = delete;
    static PageCache _instance;
public:
    static PageCache* GetInstance()
    {
        return &_instance;
    }
public:
    //获取一个k页的span
    Span* NewSpan(size_t k);
};
#endif