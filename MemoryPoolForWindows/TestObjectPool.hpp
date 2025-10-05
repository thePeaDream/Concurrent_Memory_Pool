#pragma once
#include "ObjectPool.hpp"

struct TreeNode
{
    int _val = 0;
    TreeNode* _left = nullptr;
    TreeNode* _right = nullptr;
    TreeNode()
        :_val(0)
        , _left(nullptr)
        , _right(nullptr)
    {
    }
};

static void TestObjectPool()
{
    //申请释放的轮次
    const size_t Rounds = 4;

    //一轮申请释放的次数
    const size_t N = 1000;

    ObjectPool<TreeNode> pool;

    std::vector<TreeNode*> v1;
    v1.reserve(N);

    size_t begin1 = clock();
    for (size_t i = 0; i < Rounds;++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            v1.push_back(new TreeNode);
        }
        for (size_t j = 0; j < N; ++j)
        {
            delete v1[j];
        }
        v1.clear();
    }
    size_t end1 = clock();

    std::vector<TreeNode*> v2;
    v2.reserve(N);
    size_t begin2 = clock();
    for (size_t i = 0; i < Rounds; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            v2.push_back(pool.New());
        }
        for (size_t j = 0; j < N; ++j)
        {
            pool.Delete(v2[j]);
        }
        v2.clear();
    }
    size_t end2 = clock();

    // printf("%lu轮下每轮申请释放%lu次，new所需时间：%lu ms\n",Rounds,N,end1 - begin1);
    // printf("%lu轮下每轮申请释放%lu次，objectNew所需时间：%lu ms\n",Rounds,N,end2 - begin2);
}
