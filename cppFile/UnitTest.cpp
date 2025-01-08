#include "../ObjectPool/ObjectPool.hpp"
#include "../hFile/MyMalloc.h"
#include "../hFile/ThreadCache/ThreadCache.h"
#include "../hFile/CentralCache/CentralCache.h"
#include "../hFile/PageCache/PageCache.h"


void TestConcurrentAlloc()
{
    void* p1 = ConcurrentAlloc((size_t)3);
    cout << p1 << endl;
    void* p2 = ConcurrentAlloc((size_t)9);
    cout << p2 << endl;
    void* p3 = ConcurrentAlloc((size_t)17);
    cout << p3 << endl;
    void* p4 = ConcurrentAlloc((size_t)3);
    cout << p4 << endl;
    void* p5 = ConcurrentAlloc((size_t)9);
    cout << p5 << endl;
    void* p6 = ConcurrentAlloc((size_t)17);
    cout << p6 << endl;
}

void TestConcurrentAlloc2()
{
    for(size_t i = 0; i < 1024; i++)
    {
        void* p1 = ConcurrentAlloc(8);
        cout << p1 << endl;
    }
    void* p2 = ConcurrentAlloc(8);
}

void TestAddressShift()
{
    //页号
    size_t id1 = 100;
    size_t id2 = 101;
    //id1对应的起始地址
    char* p1 = (char*)(id1 << PAGE_SHIFT);
    //id2对应的起始地址
    char* p2 = (char*)(id2 << PAGE_SHIFT);

    //id1页号内的所有地址
    for(size_t i = 0; i < (1 << PAGE_SHIFT); ++i)
    {
        cout << (void*)p1 << " 对应页号：" << ((long long)p1 >> PAGE_SHIFT) << endl;
        ++p1;
    }

    //id2页号内的所有地址
    // for(size_t i = 0; i < (1 << PAGE_SHIFT); ++i)
    // {
    //     cout << (void*)p2 << " 对应页号：" << ((long long)p2 >> PAGE_SHIFT) << endl;
    //     ++p2;
    // }
}
void TestConcurrentFree()
{
    void* p1 = ConcurrentAlloc((size_t)3);
    cout << p1 << endl;
    void* p2 = ConcurrentAlloc((size_t)9);
    cout << p2 << endl;
    void* p3 = ConcurrentAlloc((size_t)17);
    cout << p3 << endl;
    void* p4 = ConcurrentAlloc((size_t)3);
    cout << p4 << endl;
    void* p5 = ConcurrentAlloc((size_t)9);
    cout << p5 << endl;
    void* p6 = ConcurrentAlloc((size_t)17);
    cout << p6 << endl;
    ConcurrentFree(p1,3);
    ConcurrentFree(p2,9);
    ConcurrentFree(p3,17);
    ConcurrentFree(p4,3);
    ConcurrentFree(p5,9);
    ConcurrentFree(p6,17);
    cerr << "释放成功"<<endl;
}
int main()
{
    // TestConcurrentAlloc2();
    // TestAddressShift();
    TestConcurrentFree();
    return 0;
}