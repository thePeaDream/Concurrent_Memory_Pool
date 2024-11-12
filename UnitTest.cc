#include "ObjectPool.hpp"
#include <time.h>
#include "ConcurrentAlloc.hpp"
#include "ThreadCache.hpp"
#include <thread>


void Alloc1()
{
    for(size_t i = 0; i < 5; ++i)
    {
        void* ptr = ConcurrentAlloc(6);
    }
    cout <<"thread1:"<<pTLSThreadCache << endl;
    cout << std::this_thread::get_id() << endl;
}
void Alloc2()
{
    for(size_t i = 0; i < 5; ++i)
    {
        void* ptr = ConcurrentAlloc(7);
    }
    cout << "thread2:"<<pTLSThreadCache << endl;
    cout << std::this_thread::get_id() << endl;
}
void Alloc3()
{
    for(size_t i = 0; i < 5; ++i)
    {
        void* ptr = ConcurrentAlloc(9);
    }
    cout <<"thread3:"<<pTLSThreadCache << endl;
    cout << std::this_thread::get_id() << endl;
}


void TLSTest()
{
    //创建线程分别去执行两个不同函数
    std::thread t1(Alloc1);
    std::thread t2(Alloc2);
    std::thread t3(Alloc3);   
    t1.join();
    t2.join();
    t3.join();
}
 
void testObjectPool()
{
    static int number = 100000000; 
    ObjectPool<int>* pool = new ObjectPool<int>();
    //定长内存池 和 普通内存池效率对比
    //new底层调用的operator new()就是malloc实现的
    clock_t begin = clock();
    for(int i = 0;i < number; ++i)
    {
        int * ptr = pool->New();
        pool->Delete(ptr);
    }
    clock_t end = clock();
    cout << "ObjectPool:"<< end - begin << endl;

    begin =clock();
    for(int i = 0;i < number; ++i)
    {
        int * ptr = new int;
        delete ptr;
    }
    end = clock();
    cout << "MallocPool:"<< end - begin << endl;
}



int main()
{
    //testObjectPool();
    //TLSTest();
    
    return 0;
}