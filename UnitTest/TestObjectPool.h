#pragma once
#include "../Common/ObjectPool.hpp"

static void TestObjectPool()
{
    ObjectPool<int> pool;
    int* p1 = pool.New();
    *p1 = 100;
    int* p2 = pool.New();
    *p2 = 200;
    cout << p1 << " : " << *p1 << endl;
    cout << p2 << " : " << *p2 << endl;
    pool.Delete(p1);
    pool.Delete(p2);
    cout << "正常释放" <<endl;

    //会重复使用上面的地址
    int* p3 = pool.New();
    *p3 = 300;
    int* p4 = pool.New();
    *p4 = 400;
    cout << p3 << ":" << *p3 << endl;
    cout << p4 << ":" << *p4 << endl;
    pool.Delete(p3);
    pool.Delete(p4);
    cout << "正常释放" <<endl;
}