#include "../../hFile/Common/MapRule.h"
#include "../../hFile/ThreadCache/FreeList.hpp"
#include "../../hFile/CentralCache/CentralCache.h"
#include "../../hFile/ThreadCache/ThreadCache.h"
#include "../../hFile/PageCache/PageCache.h"

CentralCache CentralCache::_centralCacheInstance;

//将一定数量的内存块对象(连续)返回给thread cache
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t n, size_t nsize)
{
    //1 找到对应的桶 获取内存块
    size_t index = MapRule::Index(nsize);

    _spanLists[index]._mtx.lock();
    
    //2 从当前spanList桶中获取一个非空的span，如果没有，内部会自动向page cache申请span，然后切成小块内存

    Span* span = GetOneSpan(_spanLists[index],nsize);
    assert(span);
    assert(span->_freeList);

    //3 从span->_freeList开始，获取n个内存块，可能不够n个
    start = end = span->_freeList;
    size_t i = 0;
    size_t actualNum = 1;//实际获取的内存块
    //让end往后挪，span里不够n个内存块 或者 往后移动n-1次 终止
    while(NextObj(end) != nullptr && i < n-1)
    {
        end = NextObj(end);
        i++;
        actualNum++;
    }
    span->_freeList = NextObj(end);
    NextObj(end) = nullptr;
    span->_useCount += actualNum;

    _spanLists[index]._mtx.unlock();

    return actualNum;
}

//切分成size大小内存块的span不够，计算向Page Cache获取 几页 的span
size_t CentralCache::NumPage(size_t size)
{
    //thread cache最多一次批量申请多少个内存块对象
    size_t num = ThreadCache::GetMemBlockNumberTopLimit(size);
    //一次批量申请的最大总大小 byte
    size_t sumSize = num * size;

    //若一页为8KB 2^13byte，一次批量申请的页数
    size_t pageNum = sumSize >> 13;
    if(pageNum == 0) pageNum = 1;
    return pageNum;
}

//获取一个非空的Span
Span* CentralCache::GetOneSpan(SpanList& list,size_t size)
{
    //1 查看当前spanList中是否还有空闲的span
    Span* it = list.begin();
    while(it != list.end())
    {
        if(it->_freeList)
            return it;
        it = it->_next;
    }

    //2 没有空闲的span了，需要向Page Cache申请span，同时还要手动切分成小块内存
    size_t page = NumPage(size);//通过要切分的内存块大小，确定要获取几页的span

    //先把central cache的桶锁解掉，这样如果其他线程释放内存回来，不会阻塞
    list._mtx.unlock();

    //访问PageCache时就要加锁
    PageCache::GetInstance()->_mtx.lock();
    Span* span = PageCache::GetInstance()->NewSpan(page);
    PageCache::GetInstance()->_mtx.unlock();
    
    //注意：获取的这个span，只有当前线程可以访问到，对获取的span进行切分时，可以先不加锁
    //多个线程要竞争时才需要加锁

    //3 切分span成小块内存，挂载到_freeList里
    char* start = (char*)(span->_pageId << 13);//通过页号算出管理页起始地址
    size_t sumSize = (span->_n) << 13; // 通过页数算出大块内存字节数 <<13就是 *2^13
    char* end = start + sumSize;
    span->_freeList = start;

    //当前要尾插的内存块
    char* cur = start + size;
    //已挂载内存块的尾部
    char* tail = start;
    while(cur < end)
    {
        NextObj(tail) = cur;
        tail = cur;
        cur += size;
    }
    //切分好以后，往spanlist里插入span，需要加锁，因为此时可能有其他线程在获取span
    //切好span,把span挂桶里去时，再加锁
    list._mtx.lock();
    list.PushFront(span);
    return span;
}