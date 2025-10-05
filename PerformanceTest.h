#pragma once
#include "ConcurrentMalloc.h"
#include <atomic>

//nthreads:线程数
//nrounds:轮次
//ntimes:一轮申请和释放的次数
static void PerformanceMalloc(size_t nthreads,size_t nrounds,size_t ntimes)
{
    std::vector<std::thread> vthread(nthreads);
    std::atomic<size_t> malloc_costtimes(0);
    std::atomic<size_t> free_costtimes(0);

    for(size_t k = 0; k < nthreads; ++k)
    {
        vthread[k] = std::thread([&,k](){
            std::vector<void*> v;
            v.reserve(ntimes);
            for(size_t j = 0; j < nrounds; ++j)
            {
                size_t begin1 = clock();
                for(size_t i = 0; i < ntimes; ++i)
                    v.push_back(malloc(16));
                size_t end1 = clock();

                size_t begin2 = clock();
                for(size_t i = 0; i < ntimes; ++i)
                    free(v[i]);
                size_t end2 = clock();
                v.clear();

                malloc_costtimes += (end1 - begin1);
                free_costtimes += (end2 - begin2);
            }
        });
    }
    for(auto& t: vthread)
    {
        t.join();
    }
    cout << nthreads << "个线程并发执行" << nrounds << "轮," << "每轮malloc " << ntimes << "次，花费时间:" << malloc_costtimes.load() << " ms" << endl;
    cout << nthreads << "个线程并发执行" << nrounds << "轮," << "每轮free " << ntimes << "次，花费时间:" << free_costtimes.load() << " ms" << endl;
    cout << nthreads << "个线程并发malloc&&free累计" << ntimes*nthreads*nrounds << "次，花费时间:" << malloc_costtimes.load()+free_costtimes.load() << " ms" << endl;    
}

static void PerformanceConcurrentMalloc(size_t nthreads,size_t nrounds,size_t ntimes)
{
    std::vector<std::thread> vthread(nthreads);
    std::atomic<size_t> malloc_costtimes(0);
    std::atomic<size_t> free_costtimes(0);

    for(size_t k = 0; k < nthreads; ++k)
    {
        vthread[k] = std::thread([&,k](){
            std::vector<void*> v;
            v.reserve(ntimes);
            for(size_t j = 0; j < nrounds; ++j)
            {
                size_t begin1 = clock();
                for(size_t i = 0; i < ntimes; ++i)
                    v.push_back(ConcurrentMalloc(16));
                size_t end1 = clock();

                size_t begin2 = clock();
                for(size_t i = 0; i < ntimes; ++i)
                    ConcurrentFree(v[i]);
                size_t end2 = clock();
                v.clear();

                malloc_costtimes += (end1 - begin1);
                free_costtimes += (end2 - begin2);
            }
        });
    }
    for(auto& t: vthread)
    {
        t.join();
    }

    cout << nthreads << "个线程并发执行" << nrounds << "轮," << "每轮ConcurrentMalloc " << ntimes << "次，花费时间:" << malloc_costtimes.load() << " ms" << endl;
    cout << nthreads << "个线程并发执行" << nrounds << "轮," << "每轮ConcurrentFree " << ntimes << "次，花费时间:" << free_costtimes.load() << " ms" << endl;
    cout << nthreads << "个线程并发ConcurrentMalloc&&ConcurrentFree累计" << ntimes*nthreads*nrounds << "次，花费时间:" << malloc_costtimes.load()+free_costtimes.load() << " ms" << endl;
}

static void PerformanceTest()
{
    size_t nthreads = 4;
    size_t nrounds = 4;
    size_t ntimes = 1000;
    PerformanceMalloc(nthreads,nrounds,ntimes);
    std::cout << std::endl << std::endl;
    PerformanceConcurrentMalloc(nthreads,nrounds,ntimes);
}