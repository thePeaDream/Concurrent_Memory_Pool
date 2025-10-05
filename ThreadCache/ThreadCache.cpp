#include "ThreadCache.hpp"
#include "../CentralCache/CentralCache.hpp"
// thread_local ThreadCache* pTLSThreadCache = nullptr;
void* ThreadCache::Allocate(size_t size)
{
    assert(size <= MAX_SIZES);
    assert(size > 0);
    //对齐后的内存块大小
    size_t alignSize = AlignMap::Align(size);
    //映射的桶位置
    size_t index = AlignMap::Mapping(size);

    //查看对应桶/自由链表是否为空，为空先向CentralCache获取一批小内存块对象
    if(_freeLists[index].Empty())
        FetchObjectFromCentralCache(index,alignSize);

    //从自由链表中获取小内存块对象
    void* obj = _freeLists[index].Pop();
    return obj;
}
void ThreadCache::Deallocate(void* obj,size_t size)
{
    assert(size <= MAX_SIZES);
    assert(size > 0);
    //1 对齐内存块大小 找到要映射的自由链表桶
    size_t alignSize = AlignMap::Align(size);
    size_t index = AlignMap::Mapping(size);
    //2 将内存块对象头插入到对应自由链表中
    _freeLists[index].Push(obj);
    //3 检查自由链表的长度，过长需要将部分/全部内存块对象还回给CentralCache
    CheckListLength(_freeLists[index],alignSize);
}
void ThreadCache::FetchObjectFromCentralCache(size_t index,size_t alignSize)
{
    void* start = nullptr;
    void* end = nullptr;
    //自由链表/桶小内存块对象不足时，一次向CentralCache申请多少个小内存块对象
    //采用慢增长
    size_t limitNum = ApplyNumberLimit(alignSize);
    size_t allocSize = _freeLists[index].AllocSize();
    size_t batchNum = limitNum < allocSize ? limitNum:allocSize;
    if(batchNum == allocSize)
        _freeLists[index].AllocSize()++;
    
    //实际申请到的内存块数量
    size_t actualNum =  CentralCache::GetInstance()->FetchRangeObject(start,end,batchNum,alignSize);
    assert(actualNum >0);
    _freeLists[index].PushRange(start,end,actualNum);
}
void ThreadCache::CheckListLength(FreeList& freelist,size_t alignSize)
{
    if(freelist.Size() > freelist.AllocSize())
    {
        void* start = nullptr;
        void* end = nullptr;
        freelist.PopRange(start,end,freelist.AllocSize());
        CentralCache::GetInstance()->ReleaseListToSpans(start,alignSize);
    }
}
