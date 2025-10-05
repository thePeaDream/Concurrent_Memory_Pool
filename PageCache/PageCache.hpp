#ifndef __PAGECACHE__
#define __PAGECACHE__
#include "../Common/AlignMap.hpp"
#include "../Common/Span.hpp"
#include "../Common/Common.h"
#include "../Common/SystemAllocFree.h"
#include "../Common/ObjectPool.hpp"
#include "PageTable.hpp"

class PageCache
{
public:
    static const size_t NPAGES = 128; 
    std::mutex _mtx;
private:
    //第0个占位
    SpanList _spanLists[NPAGES + 1];
    //unordered_map<PAGE_ID,Span*> _mapIdToSpan;
    PageTable<64 - PAGESHIFT> _mapIdToSpan;
    ObjectPool<Span> _objectPool;
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

    //根据内存块对象，找到对应的Span*
    Span* ObjectToSpan(void* obj);
    
    //CentralCache还span对象给PageCache
    void ReleaseSpanToPageCache(Span* span);
};
#endif