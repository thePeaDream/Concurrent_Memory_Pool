#pragma once
//公共头文件
#include <iostream>
#include <mutex>
#include <thread>

#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
//申请<= MAX_BYTES的内存块，都直接向Thread Cache申请
static const size_t MAX_BYTES = 256*1024;
//Page Cache有(NPAGES - 1)个桶，Span最多管理(NPAGES - 1)页
static const size_t NPAGES = 129;