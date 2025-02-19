#pragma once
#include "Common.h"
//对象大小 和 自由链表桶数组 对齐映射规则
class MapRule
{
public:
    //申请的对象内存块大小，对齐以后是多大
    static size_t Align(size_t size)
    {
        //整体控制在10%左右的内碎片浪费 申请的空间低于256KB的，都向Thread Cache申请
        // [1,128byte]             8byte对齐           freeList[0,16)
        // [129byte,1024byte]      16byte对齐          freeList[16,72)
        // [1025byte,8*1024]       128byte对齐         freeList[72,128)
        // [8*1024+1,64*1024]      1024byte对齐        freeList[128,184)
        // [64*1024+1,256*1024]    8*1024byte对齐      freeList[184,208)
        if(size <= 128)
        {
            return _align(size,8);
        }
        else if(size <= 1024)
        {
            return _align(size,16);
        }
        else if(size <= 8*1024)
        {
            return _align(size,128);
        }
        else if(size <= 64*1024)
        {
            return _align(size,1024);
        }
        else if(size <= 256*1024)
        {
            return _align(size,8*1024);
        }
        else
        {
            return _align(size,1<<PAGE_SHIFT);//按页对齐
        }
    }
    //映射到哪一个自由链表桶里 / 该向哪个自由链表取内存块
    static size_t Index(size_t size)
    {
        //每个区间有多少个自由链表
        static int group_array[4] = {16, 56, 56, 56};
        if(size <= 128){
            return _index(size,3);
        }
        else if(size <= 1024){
            return group_array[0]+_index(size-128,4);
        }
        else if(size <= 8*1024){
            return group_array[0]+group_array[1]+_index(size-1024,7);
        }       
        else if(size <= 64*1024){
            return group_array[0]+group_array[1]+group_array[2]+_index(size-8*1024,10); 
        }
        else if(size <= 256*1024){
            return group_array[0]+group_array[1]+group_array[2]+group_array[3]+_index(size-64*1024,13); 
        }
        else{
            assert(false);
            return -1;
        }
    }
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
