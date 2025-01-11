#pragma once
#include "ThreadCache/ThreadCache.h"


//向Thread Cache申请size byte 空间
void* ConcurrentAlloc(size_t size);

//把内存块对象还给Thread Cache
void ConcurrentFree(void* ptr);
