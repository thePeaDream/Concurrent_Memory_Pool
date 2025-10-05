#pragma once
#include "../Common/SystemAllocFree.h"
static void TestSystemAllocFree()
{
    //申请不同页数
    const size_t test_pages[] = {1,2,4,8,16};
    //申请的不同内存块起始地址
    vector<void*> allocated_blocks;
    for(size_t pages:test_pages)
    {
        void* ptr = SystemAlloc(pages);
        assert(ptr);
        allocated_blocks.push_back(ptr);
        // printf("分配%lu页成功，起始地址是：%p，大小是：%lu字节\n",pages,ptr,pages << PAGESHIFT);
    }
    cout << "测试分配内存成功" << endl;

    for(size_t i = 0; i < allocated_blocks.size(); ++i)
    {
        SystemFree(allocated_blocks[i],test_pages[i]);
        cout << "释放内存："<< allocated_blocks[i] << "成功"<<endl;
    }
    cout << "测试释放内存成功" <<endl;
}