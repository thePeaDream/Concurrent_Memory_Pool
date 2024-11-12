#pragma once
#include "Common.h"
//表示申请的内存块，大小都是 N byte
// template<size_t N>
// class ObjectPool{
// public:  
// };

//针对申请T类型对象的内存池 —— 定长内存池
template<class T>
class ObjectPool
{
public:
    //会有越界访问的问题，_memory一直+,最后会越界访问
    //空间用完，就会异常，我怎么知道空间用完了？->定义一个_leftByte记录还剩余多少内存
    //版本1
    // T* New()
    // {
    //     if(_memory == nullptr)
    //     {
    //         _memory = (char*)malloc(128*1024);
    //         if(_memory == nullptr)
    //             throw bad_alloc();
    //     }
    //     T* obj = (T*)_memory;
    //     _memory += sizeof(T);
    //     return obj;
    // }

    //版本2
    //如果申请的空间大小，无法整除T的大小，那么不断申请空间，最后会剩下一小块，这一小块不够一个对象使用，所以_leftBytes < sizeof(T)
    //剩下不够一个对象的字节数直接丢弃，因为我要的每一个小块内存至少是一个T对象大小
    // T* New()
    // {
    //     if(_leftBytes == 0)//当大块内存块没有剩余空间时，重新开一块内存
    //     {
    //         //和顺序表不一样，不是增容，是这块内存已经被切走/申请完了，切成小块给别人用了，还回来也是到自由链表freeList里面
    //         //所以再去开一块空间给_memory
    //         _memory = (char*)malloc(128*1024);
    //         if(_memory == nullptr)
    //             throw bad_alloc();
    //         _leftBytes = 128*1024;
    //     }
    //     T* obj = (T*)_memory;
    //     _memory += sizeof(T);
    //     _leftBytes -= sizeof(T);
    //     return obj;
    // }

    //版本3
    //之前还回来的内存块，应该优先给用户使用,还回来的内存块，重复利用
    // T* New()
    // {
    //     if(_leftBytes < sizeof(T))//当剩余内存不够一个对象大小时，则重新开一块内存
    //     {
    //         //和顺序表不一样，不是增容，是这块内存已经被切走/申请完了，切成小块给别人用了，还回来也是到自由链表freeList里面
    //         //所以再去开一块空间给_memory
    //         _memory = (char*)malloc(128*1024);
    //         if(_memory == nullptr)
    //             throw bad_alloc();
    //         _leftBytes = 128*1024;
    //     }
    //     T* obj = (T*)_memory;
    //     _memory += sizeof(T);
    //     _leftBytes -= sizeof(T);
    //     return obj;
    // }


    //版本1
    //32位下，指针大小是4个字节。64位下，指针大小是8个字节！！！！
    //如何保证看内存块 指针大小的字节数? *(void**)_freeList
    // void Delete(T* obj)
    // {
    //     //内存块的前4/8字节指向下一个内存块的地址
    //     if(_freeList == nullptr)
    //     {
    //         _freeList = obj;
    //         //32位下，没有问题。64位下，指针大小是8个字节！！！！
    //         //(int*)obj -> int*解引用，就看内存块的前4个字节 
    //         *(int*)obj = nullptr; 
    //     }
    // }

    //版本2
    //_freeList不管是否为空，都能统一用一种方式
    // void Delete(T* obj)
    // {
    //     //内存块的前4/8字节指向下一个内存块的地址
    //     if(_freeList == nullptr)
    //     {
    //         _freeList = obj;
    //         //(int*)obj -> int*解引用，就取内存块的前 4(int大小) 个字节 
    //         //*(int*)obj = nullptr;
            
    //         //(void**)obj -> void**解引用，就取内存块的前 void*大小 个字节
    //         *(void**)obj = nullptr; 
    //     }
    //     //自由链表不为空，就把还回来的内存块依次头插，效率一定比尾插高
        
    //     //让obj的前4/8字节存储下一个内存块的地址
    //     *(void**)obj = _freeList;
    //     //链表头换人
    //     _freeList = obj;
    // }

    //版本3
    //要手动析构
    // void Delete(T* obj)
    // {
    //     //头插 用户释放的内存块
    //     //1 让内存块的前4/8字节指向下一个内存块地址
    //     //2 让obj成为头部
    //     *(void**)obj = _freeList;
    //     _freeList = obj;
    // }

    //版本4
    //如果T的大小 < 指针的大小，那就让一个内存块的大小 = 指针大小,这样在自由链表里，不管一个对象是多大，一定能存下一个指针
    // T* New()
    // {
    //     T* obj = nullptr;
    //     //优先把还回来的内存块，从头开始，重复利用
    //     if(_freeList)
    //     {
    //         //_freeList的头4/8字节就存着下一个被释放内存块的位置
    //         void* next = *(void**)_freeList;
    //         //_freeList的头部开始，重复利用
    //         obj = _freeList;

    //         _freeList = next;
    //     }
    //     else
    //     {
    //         if(_leftBytes < sizeof(T))//当剩余内存不够一个对象大小时，则重新开一块内存
    //         {
    //             //和顺序表不一样，不是增容，是这块内存已经被切走/申请完了，切成小块给别人用了，还回来也是到自由链表freeList里面
    //             //所以再去开一块空间给_memory
    //             _memory = (char*)malloc(128*1024);
    //             if(_memory == nullptr)
    //                 throw bad_alloc();
    //             _leftBytes = 128*1024;
    //         }
    //         obj = (T*)_memory;
    //         _memory += sizeof(T);
    //         _leftBytes -= sizeof(T);
    //     }
       
    //     return obj;
    // }
    
    //版本5
    //要对已有的空间，自定义类型对象手动初始化 —— 定位new
    // T* New()
    // {
    //     T* obj = nullptr;
    //     //优先把还回来的内存块，从头开始，重复利用
    //     if(_freeList)
    //     {
    //         //_freeList的头4/8字节就存着下一个被释放内存块的位置
    //         void* next = *(void**)_freeList;
    //         //_freeList的头部开始，重复利用
    //         obj = _freeList;

    //         _freeList = next;
    //     }
    //     else
    //     {
    //         if(_leftBytes < sizeof(T))//当剩余内存不够一个对象大小时，则重新开一块内存
    //         {
    //             //和顺序表不一样，不是增容，是这块内存已经被切走/申请完了，切成小块给别人用了，还回来也是到自由链表freeList里面
    //             //所以再去开一块空间给_memory
    //             _memory = (char*)malloc(128*1024);
    //             if(_memory == nullptr)
    //                 throw bad_alloc();
    //             _leftBytes = 128*1024;
    //         }
    //         obj = (T*)_memory;

    //         //如果T对象的大小<一个指针大小，那么存一个T对象的空间就设为 指针大小
            
    //         size_t objSize = sizeof(T) < sizeof(void*)?sizeof(void*):sizeof(T);
            
    //         _memory += objSize;
    //         _leftBytes -= objSize;
    //     }
    //     return obj;
    // }
    
    //版本6
    T* New()
    {
        T* obj = nullptr;
        //优先把还回来的内存块，从头开始，重复利用
        if(_freeList)
        {
            //_freeList的头4/8字节就存着下一个被释放内存块的位置
            void* next = *(void**)_freeList;
            //_freeList的头部开始，重复利用
            obj = (T*)_freeList;

            _freeList = next;
        }
        else
        {
            if(_leftBytes < sizeof(T))//当剩余内存不够一个对象大小时，则重新开一块内存
            {
                //和顺序表不一样，不是增容，是这块内存已经被切走/申请完了，切成小块给别人用了，还回来也是到自由链表freeList里面
                //所以再去开一块空间给_memory
                _memory = (char*)systemAlloc(128*1024);
                if(_memory == nullptr)
                    throw std::bad_alloc();
                _leftBytes = 128*1024;
            }
            obj = (T*)_memory;

            //如果T对象的大小<一个指针大小，那么存一个T对象的空间就设为 指针大小
            
            size_t objSize = sizeof(T) < sizeof(void*)?sizeof(void*):sizeof(T);
            
            _memory += objSize;
            _leftBytes -= objSize;
        }

        //定位new,显示调用T的构造函数初始化
        new(obj)T;
        return obj;
    }

    //版本4
    void Delete(T* obj)
    {
        //显示调用析构函数，清理对象
        obj->~T();

        //头插 用户释放的内存块
        //1 让内存块的前4/8字节指向下一个内存块地址
        //2 让obj成为头部
        *(void**)obj = _freeList;
        _freeList = obj;
    }
    
    void* systemAlloc(size_t size)
    {
        void* ret;
        ret = mmap(nullptr,size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
        if(ret == MAP_FAILED)
            return nullptr;
        return ret;
    }
private:
    //可以分配的大块内存块起始位置
    char* _memory = nullptr; //char*方便切内存块
    //大块内存剩下可分配的字节数
    size_t _leftBytes = 0;
    //指向释放回的内存块起始地址，放到自由链表
    void* _freeList = nullptr;
};