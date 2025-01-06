#pragma once
//定长内存池实现
#include <iostream>
#include <unistd.h>
using namespace std;
static size_t OnceAlloc = 1024 * 8; //一次向系统申请的空间为8KB

template<class T>
class ObjectPool
{
public:
    T* New()
    {
        //保证申请的对象大小至少跟指针一样大
        size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*):sizeof(T);
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
                _memory = myMalloc(OnceAlloc);
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
        //用brk向系统申请

        //当前数据段末尾
        void* old_brk = sbrk(0);
        //要设置的数据段新末尾
        void* new_brk = old_brk + size;
        //用brk函数来调整堆区最高位置
        if (brk(new_brk) == -1)
        {
            return nullptr;
        }
        //此时，old_brk和new_brk之间的空间就是新申请到的内存空间
        return old_brk;
    }
private:
    char* _memory;//存储向系统申请的大块内存
    size_t _leftSpace;//大块内存剩余可分配空间
    void* _freeList;//自由链表，管理用户释放回来的内存块对象，用链表管理起来
};