#include "../hFile/ThreadCache.h"
#include "../hFile/CentralCache.h"

void* ThreadCache::Allocate(size_t size)
{
    //小于MAX_BYTES的空间申请向ThreadCache
    assert(size <= MAX_BYTES && size > 0);
    //对齐以后实际分配的内存块大小
    size_t alignSize = MapRule::Align(size);

    //对应的自由链表桶下标
    size_t index = MapRule::Index(size);

    
    if(!_freeLists[index].Empty())
        return _freeLists[index].Pop();
    else
        return FetchFromCentralCache(index,alignSize);
}

void* ThreadCache::FetchFromCentralCache(size_t index,size_t alignSize)
{
    //慢开始反馈调节算法
    //给freeList添加一个成员maxSize，代表一次性申请内存块对象的数量上限
    
    //一开始不会向central cache批量要太多，可能用不完
    //如果不断有这个alignSize大小的内存申请需求，那么每次申请的数量都会增加，直到最大上限值
    size_t limit = GetMemBlockNumberTopLimit(alignSize);
    //实际申请的数量
    size_t actualNum = _freeLists[index].MaxSize() < limit ? _freeLists[index].MaxSize():limit;
    if(actualNum == _freeLists[index].MaxSize())
        _freeLists[index].MaxSize() += 1;
    void* start = nullptr;
    void* end = nullptr;

    //实际获取的内存块数量

    size_t actualGetNum = CentralCache::GetInstance()->FetchRangeObj(start,end,actualNum,alignSize);

    assert(actualGetNum >= 1 );

    if(start == end)
        return start;
    else
    {
        _freeLists[index].PushRange(NextObj(start),end,actualGetNum-1);
        return start;
    }
}

void ThreadCache::Deallocate(void* ptr,size_t size)
{
    assert(ptr);
    //还回来的内存块放到哪个自由链表桶里
    size_t index = MapRule::Index(size);
    _freeLists[index].Push(ptr);

    //如果当前桶长度(增加size成员) 超过 当前桶一次向Central Cache批量申请的内存块数量
    //释放内存块给Central Cache
    if(_freeLists[index].Size()>= _freeLists[index].MaxSize())
    {
        FreeMemblockToCentralCache(_freeLists[index],size);
    }
}

void ThreadCache::FreeMemblockToCentralCache(FreeList& list,size_t size)
{
    void* start = nullptr;
    void* end = nullptr;
    //将桶里头MaxSize个内存块删除，start end作为输出型参数获取
    list.PopRange(start,end,list.MaxSize());
    CentralCache::GetInstance()->ReleaseMemBlockToSpans(start,size);
}

