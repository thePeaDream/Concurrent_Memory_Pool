#include "../hFile/Common/MapRule.h"
#include "../hFile/ThreadCache/FreeList.hpp"
#include "../hFile/CentralCache/CentralCache.h"
#include "../hFile/ThreadCache/ThreadCache.h"
#include "../hFile/PageCache/PageCache.h"

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
    while(i < n - 1&&NextObj(end) != nullptr)
    {
        end = NextObj(end);
        i++;
        actualNum++;
    }
    span->_freeList = NextObj(end);
    span->_useCount += actualNum;

    NextObj(end) = nullptr;

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
        if(it == nullptr)
        {
            int tt = 0;
        }
        if(it->_freeList != nullptr)
        {
            return it;
        }
        it = it->_next;
    }
    cout << "要向PageCache申请新Span插入"<<endl;

    //2 没有空闲的span了，需要向Page Cache申请span，同时还要手动切分成小块内存
    size_t page = NumPage(size);//通过要切分的内存块大小，确定要获取几页的span

    //先把central cache的桶锁解掉，这样如果其他线程释放内存回来，不会阻塞
    list._mtx.unlock();

    //访问PageCache时就要加大锁
    PageCache::GetInstance()->_mtx.lock();
    Span* span = PageCache::GetInstance()->NewSpan(page);
  
    span->_is_use = true;
    span->_objSize = size;
    span->_n = page;
    span->_freeList = nullptr;
    PageCache::GetInstance()->_mtx.unlock();
    
    //注意：获取的这个span，只有当前线程可以访问到，对获取的span进行切分时，可以先不加锁
    //多个线程要竞争时才需要加锁

    //3 切分span成小块内存，挂载到_freeList里
    char* start = (char*)((span->_pageId) << 13);//通过页号算出管理页起始地址

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
    NextObj(tail) = nullptr;
    //切分好以后，往spanlist里插入span，需要加锁，因为此时可能有其他线程在获取span
    //切好span,把span挂桶里去时，再加锁
    list._mtx.lock();
    list.PushFront(span);
    cout << "插入了一个span"<<endl;
    return span;
}





//Thread Cache调用它还内存块给Span
// void CentralCache::ReleaseMemBlockToSpans(void* start,size_t size)
// {
//     //先算出属于哪一个桶
//     size_t index = MapRule::Index(size);
//     _spanLists[index]._mtx.lock();
//     void* cur = start;

//     while(cur)
//     {
//         //将下一个内存块对象记录下来
//         void* next = NextObj(cur);

//         //找到当前内存块对象对应的Span*
//         Span* span = PageCache::GetInstance()->MemBlockToSpan(cur);

//         //将当前内存块对象头插进对应Span
//         NextObj(cur) = span->_freeList;
//         span->_freeList = cur;

//         //设置该span被使用的内存块
//         span->_useCount--;
//         if(span->_useCount == 0)
//         {
//             //说明该span切分出去的内存块都回来了，可以回收给PageCache，
//             //PageCache去做前后页的合并，合并成更大页的Span，缓解内存碎片问题
//             _spanLists[index].Erase(span);
//             span->_freeList = nullptr;
//             span->_next = nullptr;
//             span->_prev = nullptr;
//             span->_is_use = false;
//             span->_objSize = 0;
//             //唯一不能动的是页号和页数

//             //解桶锁，让其他线程的Thread Cache也能在这个桶里申请/释放内存块，当前线程要还Span给PageCache,暂时不会访问这个桶
//             _spanLists[index]._mtx.unlock();
//             //将span返回给PageCache
//             PageCache::GetInstance()->_mtx.lock();
//             PageCache::GetInstance()->ReleaseSpanToPageCache(span);
//             PageCache::GetInstance()->_mtx.unlock();
//             //重新加上桶锁
//             _spanLists[index]._mtx.lock();
//         }
//         cur = next; 
//     }
//     _spanLists[index]._mtx.unlock();
// }