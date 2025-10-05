#pragma once
#include "AlignMap.hpp"
//[1 128] 按8byte对齐  [0,16)
//[128+1, 1024] 按16byte对齐 [16,72)
//[1025,8*1024] 按128byte对齐[72,128)
//[8*1024+1,64*1024] 按1024byte对齐[128,184)
//[64*1024+1,256*1024] 按8*1024byte对齐[184,208)  
// > 256*1024，按页对齐

static void TestAlignMap()
{
    for (size_t i = 1; i <= 128; ++i)
    {
        size_t alignSize = AlignMap::Align(i);
        assert(alignSize == ((i + 7) & ~7)); //向上取最小的，能够整除8的数
    }
    for (size_t i = 129; i <= 1024; ++i)
    {
        size_t alignSize = AlignMap::Align(i);
        assert(alignSize == ((i + 15) & ~15));
    }

    assert(AlignMap::Mapping(10) == 1);
    assert(AlignMap::Mapping(128) == 15);
    assert(AlignMap::Mapping(8 * 1024) == 127);
}