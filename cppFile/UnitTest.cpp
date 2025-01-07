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
int main()
{
    TestConcurrentAlloc2();
    return 0;
}