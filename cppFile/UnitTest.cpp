#include "../hFile/MyMalloc.h"
#include <iostream>


// void TestConcurrentAlloc()
// {
//     void* p1 = ConcurrentAlloc((size_t)3);
//     cout << p1 << endl;
//     void* p2 = ConcurrentAlloc((size_t)9);
//     cout << p2 << endl;
//     void* p3 = ConcurrentAlloc((size_t)17);
//     cout << p3 << endl;
//     void* p4 = ConcurrentAlloc((size_t)3);
//     cout << p4 << endl;
//     void* p5 = ConcurrentAlloc((size_t)9);
//     cout << p5 << endl;
//     void* p6 = ConcurrentAlloc((size_t)17);
//     cout << p6 << endl;
// }

// void TestConcurrentAlloc2()
// {
//     for(size_t i = 0; i < 1024; i++)
//     {
//         void* p1 = ConcurrentAlloc(8);
//         cout << p1 << endl;
//     }
//     void* p2 = ConcurrentAlloc(8);
// }

// void TestAddressShift()
// {
//     //页号
//     size_t id1 = 100;
//     size_t id2 = 101;
//     //id1对应的起始地址
//     char* p1 = (char*)(id1 << PAGE_SHIFT);
//     //id2对应的起始地址
//     char* p2 = (char*)(id2 << PAGE_SHIFT);

//     //id1页号内的所有地址
//     for(size_t i = 0; i < (1 << PAGE_SHIFT); ++i)
//     {
//         cout << (void*)p1 << " 对应页号：" << ((long long)p1 >> PAGE_SHIFT) << endl;
//         ++p1;
//     }

//     //id2页号内的所有地址
//     // for(size_t i = 0; i < (1 << PAGE_SHIFT); ++i)
//     // {
//     //     cout << (void*)p2 << " 对应页号：" << ((long long)p2 >> PAGE_SHIFT) << endl;
//     //     ++p2;
//     // }
// }
// void TestConcurrentFree()
// {
//     void* p1 = ConcurrentAlloc((size_t)3);
//     cout << p1 << endl;
//     void* p2 = ConcurrentAlloc((size_t)9);
//     cout << p2 << endl;
//     void* p3 = ConcurrentAlloc((size_t)17);
//     cout << p3 << endl;
//     void* p4 = ConcurrentAlloc((size_t)3);
//     cout << p4 << endl;
//     void* p5 = ConcurrentAlloc((size_t)9);
//     cout << p5 << endl;
//     void* p6 = ConcurrentAlloc((size_t)17);
//     cout << p6 << endl;
//     ConcurrentFree(p1);
//     ConcurrentFree(p2);
//     ConcurrentFree(p3);
//     ConcurrentFree(p4);
//     ConcurrentFree(p5);
//     ConcurrentFree(p6);
//     cerr << "释放成功"<<endl;
// }


// void TestBetchmarkOnlyConcurrentMalloc(size_t ntimes,size_t nthreads,size_t rounds)
// {
//     std::vector<std::thread> vthread(nthreads);
//     std::atomic<size_t> malloc_time(0);
    
//     for(size_t i = 0; i < nthreads;++i)
//     {
//         vthread[i] = std::thread(
//             [&,i](){
//                 for(size_t j = 0; j < rounds; ++j)
//                 {
//                     size_t begin1 = clock();
//                     for(size_t z = 0; z < ntimes; ++z)
//                     {
//                         void* ptr = ConcurrentAlloc((16 + z) % 8192 + 1);
//                     }
//                     size_t end1 = clock();                            
//                     malloc_time += (end1-begin1);
//                 }

//             });
//     }
//     for(auto& t : vthread)
//     {
//         t.join();
//     }
//     printf("%lu个线程并发执行%lu轮次，每轮申请%lu次，花费%lu ms\n",nthreads,rounds,ntimes,malloc_time.load());
// }


//ntimes 一轮申请和释放内存的次数
//round 跑多少轮
//nworks 线程数
void BenchmarkMalloc(size_t ntimes, size_t nworks, size_t rounds)
{
	vector<std::thread> vthread(nworks);
	atomic<size_t> malloc_costtime(0);
	atomic<size_t> free_costtime(0);
	
	for (size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = thread([&, k]() {
			vector<void*> v;
			v.reserve(ntimes);
			for (size_t j = 0; j < rounds; ++j)
			{
				size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					//v.push_back(malloc(16));
					v.push_back(malloc((16+i)%8192+1));
				}
				size_t end1 = clock();

				size_t begin2 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					free(v[i]);
				}
				size_t end2 = clock();
				v.clear();

				malloc_costtime += (end1 - begin1);
				free_costtime += (end2 - begin2);
			}
			});
	}
	for (auto& t : vthread)
	{
		t.join();
	}
	printf("%u个线程并发执行%u轮次，每轮申请%u次，花费%u ms\n", nworks, rounds, ntimes, malloc_costtime.load());
	printf("%u个线程并发执行%u轮次，每轮释放%u次，花费%u ms\n", nworks, rounds, ntimes, free_costtime.load());
	printf("%u个线程并发执行%u轮次，每轮申请&释放%u次，总计花费%u ms\n", nworks, rounds, ntimes, malloc_costtime.load()+free_costtime.load());
}

void BenchmarkConcurrentMalloc(size_t ntimes, size_t nworks, size_t rounds)
{
	vector<std::thread> vthread(nworks);
	atomic<size_t> malloc_costtime(0);
	atomic<size_t> free_costtime(0);

	for (size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = thread([&, k]() {
			vector<void*> v;
			v.reserve(ntimes);
			for (size_t j = 0; j < rounds; ++j)
			{
				size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					//v.push_back(ConcurrentAlloc(16));
					v.push_back(ConcurrentAlloc((16 + i) % 8192 + 1));
				}
				size_t end1 = clock();

				size_t begin2 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					ConcurrentFree(v[i]);
				}
				size_t end2 = clock();
				v.clear();

				malloc_costtime += (end1 - begin1);
				free_costtime += (end2 - begin2);
			}
			});
	}
	for (auto& t : vthread)
	{
		t.join();
	}
	printf("%u个线程并发执行%u轮次，每轮申请%u次，花费%u ms\n", nworks, rounds, ntimes, malloc_costtime.load());
	printf("%u个线程并发执行%u轮次，每轮释放%u次，花费%u ms\n", nworks, rounds, ntimes, free_costtime.load());
	printf("%u个线程并发执行%u轮次，每轮申请&释放%u次，总计花费%u ms\n", nworks, rounds, ntimes, malloc_costtime.load() + free_costtime.load());
}


int main()
{
    // TestConcurrentAlloc2();
    // TestAddressShift();
    // TestConcurrentFree();
	BenchmarkMalloc(10000, 3, 3);
	cout << endl << endl;
	BenchmarkConcurrentMalloc(10000, 3, 3);
	return 0;
}