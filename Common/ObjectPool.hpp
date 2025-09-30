#pragma once
#include "Common.h"
#include "SystemAllocFree.h"
//定长内存池
//不考虑内存碎片问题，效率高
template <class T>
class ObjectPool
{
private:
    //大内存块还没有被切分的起始地址
    char* _memory = nullptr;
    //大内存块剩余空间大小
    size_t _remainSize = 0;
    //自由链表管理还回来的小内存块对象
    void* _freeList = nullptr;
public:
    T* New()
    {
        //1 首先看看自由链表里还有没有空闲的小内存块对象
        if(_freeList)
        {
            T* obj = (T*)_freeList;
            _freeList = *(void**)_freeList;

            //定位new调用默认构造函数初始化
            new(obj)T();
            return obj;
        }
        //2 剩余空间不够一个内存块对象大小时，要向系统申请一块堆空间，否则直接切割即可
        //注意要让一个小内存块的大小，至少大于一个指针大小，用来让前8Byte/4Byte存下一个内存块对象的地址
        size_t objSize = sizeof(void*) > sizeof(T) ?sizeof(void*):sizeof(T);
        if(_remainSize < objSize || _memory == nullptr)
        {
            _remainSize = 128 * 1024;//128 KB
            _memory = (char*)SystemAlloc(_remainSize >> PAGESHIFT);
        }
        //开始切割
        T* obj = (T*)_memory;
        _memory += objSize;
        _remainSize -= objSize;
        //定位new
        new(obj)T();
        return obj;
    }
    void Delete(T* obj)
    {
        assert(obj);
        obj->~T();
        //往自由链表头插小内存块对象
        *(void**)obj = _freeList;
        _freeList = obj;
    }
};