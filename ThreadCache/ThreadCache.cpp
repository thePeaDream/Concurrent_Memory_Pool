#include "ThreadCache.hpp"
#include "../CentralCache/CentralCache.hpp"
void* ThreadCache::Allocate(size_t size)
{
    //对齐后的内存块大小
    size_t alignSize = AlignMap::Align(size);
    //映射的桶位置
    size_t index = AlignMap::Mapping(size);

    //查看对应桶/自由链表是否为空，为空先向CentralCache获取一批小内存块对象
    if(_freeLists[index].Empty())
        FetchObjectFromCentralCache(index,alignSize);

    //从自由链表中获取内存块
    void* obj = _freeLists[index].Pop();
    return obj;
}
void ThreadCache::Deallocate(void* obj,size_t size)
{

}
void ThreadCache::FetchObjectFromCentralCache(size_t index,size_t alignSize)
{
    void* start = nullptr;
    void* end = nullptr;
    //自由链表/桶小内存块对象不足时，一次向CentralCache申请多少个小内存块对象
    //采用慢增长
    size_t batchNum = std::min(_freeLists[index].AllocSize(),ApplyNumberLimit(alignSize));
    if(batchNum == _freeLists[index].AllocSize())
        _freeLists[index].AllocSize()++;
    
    //实际申请到的内存块数量
    size_t actualNum =  CentralCache::GetInstance()->FetchRangeObject(start,end,batchNum,alignSize);
    assert(actualNum >0);
    _freeLists[index].PushRange(start,end,actualNum);
}