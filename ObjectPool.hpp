#include <iostream>
#include <list>
using std::cout;
using std::endl;
//定长内存池,表示申请的内存块，大小都是 N byte
// template<size_t N>
// class ObjectPool{
// public:
// private:
// };

//初始状态：均为空
template<class T>
class ObjectPool
{
public:
    //固定大小的内存申请释放需求
    T* New()
    {
        //优先把还回来的内存块，重复利用
        if(_freeList)
        {
            //返回自由链表中首个内存块
            T* ret = (T*)_freeList;
            //头删首个内存块，即将_freeList指向下一个内存块
            _freeList = *(void**)_freeList;
            new(ret)T;
            return ret;
        }
        //如果 剩下内存 不够 一个对象大小时，重新开大块空间
        if(_leftBytes < sizeof(T))
        {
             _leftBytes = 128*1024;
            _memory = (char*)malloc(sizeof(_leftBytes));
            if(_memory == nullptr)
                throw std::bad_alloc();
        }
        else{
            T* obj = (T*)_memory;
            //自由链表中，一个对象不管有多大，但至少能存储一个指针
            size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*):sizeof(T);
            _memory += objSize;
            _leftBytes -= objSize;

            //定位new，显示调用T的构造函数初始化
            new(obj)T;
            return obj;
        }
    }
    //释放
    void Delete(T* obj)
    {
        //显示调用析构函数清理对象空间
        obj->~T();
        //以下两种情况都可以用头插解决
        *(void**)obj = _freeList;
        _freeList = obj;
        //将内存块添加到自由链表，每个内存块的前4/8字节存储下一个内存块的地址
        // if(_freeList == nullptr)
        // {
        //     //*(void**) 就是void*,解引用看一个指针大小
        //     _freeList = obj;
        //     //32位和64位通用
        //     *((void**)obj) = nullptr;
        // }
        // else{
        //     //头插
        //     //让这个对象的头4/8byte指向freeList的第一个内存块
        //     *(void**)obj = _freeList;
        //     _freeList = obj;
        // }
    }

private:
    //可以分配的内存块起始位置
    char* _memory = nullptr;
    //剩下可分配的字节数
    size_t _leftBytes = 0;
    //指向释放回的内存块起始地址，放到自由链表
    void* _freeList = nullptr;
};