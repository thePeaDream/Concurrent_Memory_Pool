#pragma once
#include "Common.h"
static const size_t BUCKET_NUM = 208; 
//对齐映射规则
//用户申请的空间大小，要对齐到某个内存块大小
//不同的内存块大小，映射到不同的桶/下标中

//[1 128] 按8byte对齐  [0,16)
//[128+1, 1024] 按16byte对齐 [16,72)
//[1025,8*1024] 按128byte对齐[72,128)
//[8*1024+1,64*1024] 按1024byte对齐[128,184)
//[64*1024+1,256*1024] 按8*1024byte对齐[184,208)  
// > 256*1024，按页对齐
class AlignMap
{
public:
    //size:都是用户申请的内存大小
    static size_t Align(size_t size)
    {
        assert(size > 0);
        size_t alignSize = size;
        if(size <= 128)
        {
            return _Align(size,8);
        }
        else if(size <= 1024)
        {
            return _Align(size,16);
        }
        else if(size <= 8*1024)
        {
            return _Align(size,128);
        }
        else if(size <= 64*1024)
        {
            return _Align(size,1024);
        }
        else if(size <= 256*1024)
        {
            return _Align(size,8*1024);
        }
        else if(size > 256*1024)
        {
            return _Align(size,PAGESIZE);
        }
        assert(false);
        return -1;
    }
    static size_t Mapping(size_t size)
    {
        //每个内存块大小区间桶的数量
        static const size_t bucket_counts[5] = {16, 56, 56, 56, 24};
        //先对齐
        size = Align(size);
        if(size <= 128)
        {
            return _Mapping(size,3);
        }
        else if(size <= 1024)
        {
            return _Mapping(size - 128,4) + bucket_counts[0];
        }
        else if(size <= 8*1024)
        {
            return _Mapping(size - 1024,7) + bucket_counts[0] + bucket_counts[1];
        }
        else if(size <= 64*1024)
        {
            return _Mapping(size - 8*1024,10)+ bucket_counts[0] + bucket_counts[1] + bucket_counts[2];
        }
        else if(size <= 256*1024)
        {
            return _Mapping(size - 64*1024,13) + bucket_counts[0] + bucket_counts[1] + bucket_counts[2] + bucket_counts[3]; 
        }
        else if(size > 256*1024)
        {
            assert(false);
            return -1;
        }
        assert(false);
        return -1;
    }
private:
    //size:要对齐的内存块大小
    //alignNum:对齐数
    static size_t _Align(size_t size,size_t alignNum)
    {
        //找到一个最小的对齐数alignSize,使得alignSize >= size,且alignSize % alignNum = 0

        //&~(alignNum - 1): 对齐掩码，用于屏蔽低位，让结果向下舍入到alignNum的倍数
        //size + alignNum - 1: 确保结果至少达到下一个对齐边界, -1保证在size本身就是alignNum的倍数下，不会越过2个对齐边界
        return (size + alignNum - 1)&~(alignNum-1);
    }
    static size_t _Mapping(size_t remainSize,size_t alignShift)
    {
        //remainSize / (1 << alignShift) - 1
        return (remainSize >> alignShift) - 1;
    }
};