#pragma once
//定长内存池实现
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
using namespace std;
static size_t OnceAlloc = 1024 * 8; //一次向系统申请的空间为8KB

template<class T>
class ObjectPool
{
public:
    T* New()
    {
        //保证申请的对象大小至少跟指针一样大
        size_t objSize = sizeof(T) > sizeof(void*) ? sizeof(T):sizeof(void*);
        if(_freeList)//优先使用用户还回来的内存块对象
        {
            T* ret = (T*)_freeList;
            _freeList = *(void**)_freeList;
            return ret;
        }
        else
        {
            if(_leftSpace < objSize)//如果剩下空间不够分配一个内存块，就向系统申请
            {
                _memory = (char*)myMalloc(OnceAlloc);
                if(_memory == nullptr)
                    throw bad_alloc();
                _leftSpace += OnceAlloc;
            }

            T* ret = (T*)_memory;
            _memory += objSize;
            _leftSpace -= objSize;

            new(ret)T;//定位new初始化
            return ret;
        }
        
    }

    void Delete(T* ptr)
    {
        if(ptr != nullptr)
        {
            //显示调用析构函数清理对象
            ptr->~T();

            //头插自由链表
            *(void**)ptr = _freeList;
            _freeList = ptr;
        }
    }
private:
    void* myMalloc(size_t size)
    {   
        //用mmap向系统申请
        void* ret = mmap(nullptr,size,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
        if(ret == MAP_FAILED) return nullptr;
        return ret;
    }

private:
    char* _memory = nullptr;//存储向系统申请的大块内存
    size_t _leftSpace = 0;//大块内存剩余可分配空间
    void* _freeList = nullptr;//自由链表，管理用户释放回来的内存块对象，用链表管理起来
};