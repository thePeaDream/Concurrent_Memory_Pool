#pragma once
#include "Common.h"
//对象大小 和 自由链表桶数组 对齐映射规则
class MapRule
{
public:
    //申请的对象内存块大小，对齐以后是多大
    static size_t Align(size_t size);
    //映射到哪一个自由链表桶里 / 该向哪个自由链表取内存块
    static size_t Index(size_t size);
private:
    //size:要申请的对象大小
    //alignNum:对齐数
    static inline size_t _align(size_t size,size_t alignNum)
    {
        // if(size % alignNum == 0)
        //     return size;
        // return size + alignNum - size % alignNum;

        //对齐数都是2^n
        //~(2^n-1)相当于将第n个二进制位后面的位全部置0，前面的位全部置1
        return (size+alignNum-1) &~(alignNum-1);
    }

    static inline size_t _index(size_t size, size_t align_shift)
    {
        // if(size % alignNum == 0)
        //     return size/alignNum - 1;
        // return size/alignNum;
        return ((size+(1<<align_shift)-1) >> align_shift) - 1;
    }
};