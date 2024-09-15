#include "ObjectPool.hpp"
#include <time.h>
int main()
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


    return 0;
}