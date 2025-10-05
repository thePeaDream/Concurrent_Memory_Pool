#pragma once
#include "../Common/Common.h"
#include "../Common/ObjectPool.hpp"

template<size_t PAGE_BITS> //2^52 或者 2^51
class PageTable
{
#if defined(_WIN64) || defined(__LP64__) //4级页表
private:
    static const size_t LEVEL1_BITS = PAGE_BITS / 4; //以13为例 2^13 * 2^3 = 2^16 = 64KB
    static const size_t LEVEL2_BITS = PAGE_BITS / 4;
    static const size_t LEVEL3_BITS = PAGE_BITS / 4;
    static const size_t LEVEL4_BITS = PAGE_BITS - LEVEL1_BITS - LEVEL2_BITS - LEVEL3_BITS;

    struct Level3
    {
        void* _table4[1 << LEVEL4_BITS];
        Level3()
        {
            memset(_table4, 0, sizeof(_table4));
        }
    };
    struct Level2
    {
        Level3* _table3[1 << LEVEL3_BITS];
        Level2()
        {
            memset(_table3, 0, sizeof(_table3));
        }
    };
    struct Level1
    {
        Level2* _table2[1 << LEVEL2_BITS];
        Level1()
        {
            memset(_table2, 0, sizeof(_table2));
        }
    };
    Level1* _table1[1 << LEVEL1_BITS];
public:
    PageTable()
    {
        memset(_table1, 0, sizeof(_table1));
    }
    void* get(PAGE_ID pageId)
    {
        size_t level1PageId = pageId >> (PAGE_BITS - LEVEL1_BITS);
        size_t level2PageId = (pageId >> (LEVEL3_BITS + LEVEL4_BITS)) & ((1 << LEVEL2_BITS) - 1);
        size_t level3PageId = (pageId >> LEVEL4_BITS) & ((1 << LEVEL3_BITS) - 1);
        size_t level4PageId = pageId & ((1 << LEVEL4_BITS) - 1);
        return _table1[level1PageId]->_table2[level2PageId]->_table3[level3PageId]->_table4[level4PageId];
    }
    void set(PAGE_ID pageId, void* ptr)
    {
        size_t level1PageId = pageId >> (PAGE_BITS - LEVEL1_BITS);
        size_t level2PageId = (pageId >> (LEVEL3_BITS + LEVEL4_BITS)) & ((1 << LEVEL2_BITS) - 1);
        size_t level3PageId = (pageId >> LEVEL4_BITS) & ((1 << LEVEL3_BITS) - 1);
        size_t level4PageId = pageId & ((1 << LEVEL4_BITS) - 1);
        if (_table1[level1PageId] == nullptr)
        {
            static ObjectPool<Level1> pool1;
            Level1* level1 = pool1.New();
            _table1[level1PageId] = level1;
        }
        if (_table1[level1PageId]->_table2[level2PageId] == nullptr)
        {
            static ObjectPool<Level2> pool2;
            Level2* level2 = pool2.New();
            _table1[level1PageId]->_table2[level2PageId] = level2;
        }
        if (_table1[level1PageId]->_table2[level2PageId]->_table3[level3PageId] == nullptr)
        {
            static ObjectPool<Level3> pool3;
            Level3* level3 = pool3.New();
            _table1[level1PageId]->_table2[level2PageId]->_table3[level3PageId] = level3;
        }
        _table1[level1PageId]->_table2[level2PageId]->_table3[level3PageId]->_table4[level4PageId] = ptr;
    }
#elif defined(_WIN32) || defined(__ILP32__)
private:
    static const size_t ARRAY_SIZE = 1 << PAGE_BITS;
    void* _array[ARRAY_SIZE];
public:
    PageTable()
    {
        memset(_array, 0, sizeof(_array));
    }
    void* get(PAGE_ID pageId)
    {
        assert((pageId >> PAGE_BITS) == 0);
        return _array[pageId];
    }
    void set(PAGE_ID pageId, void* ptr)
    {
        assert((pageId >> PAGE_BITS) == 0);
        _array[pageId] = ptr;
    }
#endif
};
