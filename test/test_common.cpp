#include "test_common.h"

#include "Common/Singleton.hpp"
#include "Common/Log.hpp"
#include "Common/String.hpp"
#include "Common/MemAllocator.hpp"


TEST_F(TestCommon,testSingle)
{
    class ClassA : public EgLab::Singleton<ClassA>
    {
    public:
        void setA(int _a) {a = _a;}
        int getA() {return a;}
    private:
        int a;
    };

    ClassA& a1 = ClassA::instance();
    a1.setA(2);
    EXPECT_EQ(a1.getA(), 2);

    ClassA& a2 = ClassA::instance();
    a2.setA(4);
    EXPECT_EQ(a1.getA(), 4);
}

TEST_F(TestCommon, Log)
{
    LOG(INFO) << "info";
    LOG(WARNING) << "test";
    LOG(ERROR) << "test";
    LOG(FATAL) << "test";
}

TEST_F(TestCommon, String)
{
    EgLab::String str;
    str.append("test");
    EXPECT_EQ(str.c_str()[0], "test"[0]);
    str.append("abcdefghijklmn");
    EXPECT_EQ(str.c_str()[0], "test"[0]);
    std::string cppstr = "testabcdefghijklmn";
    for(int i = 0 ; i < cppstr.size(); i++)
    {
        EXPECT_EQ(cppstr.c_str()[i], str.c_str()[i]);
    }

    EgLab::String a("a");
    EgLab::String b("b");
    auto c = a+b;
    EXPECT_EQ(c.c_str()[0], 'a');
    EXPECT_EQ(c.c_str()[1], 'b');
}

#include <vector>
#include <any>
#include <chrono>

TEST_F(TestCommon, anyTest)
{
    return; // just for test
    using namespace std;
    using namespace std::chrono;
    const int N = 10000000; // 测试次数

    // 准备数据
    std::vector<int> data(N);
    for (int i = 0; i < N; ++i) {
        data[i] = i;
    }

    // 测试 std::any 转换
    vector<any> anys(N);
    for (int i = 0; i < N; ++i) {
        anys[i] = data[i];
    }

    volatile int sum_any = 0; // 防止优化
    auto start_any = high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        sum_any += any_cast<int>(anys[i]);
    }
    auto end_any = high_resolution_clock::now();
    auto duration_any = duration_cast<milliseconds>(end_any - start_any).count();

    // 测试 void* 转换
    vector<void*> void_ptrs(N);
    for (int i = 0; i < N; ++i) {
        void_ptrs[i] = &data[i];
    }

    volatile int sum_void = 0; // 防止优化
    auto start_void = high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        sum_void += *(static_cast<int*>(void_ptrs[i]));
    }
    auto end_void = high_resolution_clock::now();
    auto duration_void = duration_cast<milliseconds>(end_void - start_void).count();

    cout << "std::any cost: " << duration_any << " ms, sum = " << sum_any << endl;
    cout << "void* cost: " << duration_void << " ms, sum = " << sum_void << endl;


}

TEST_F(TestCommon, MemAllocateTest)
{
    return; // the case cannot pass beacuse of the complier changed 
    EgLab::StaticSizeAllocator<int> allocator;
    int* b = static_cast<int*>(allocator.alloc());
    LOG(INFO) << b;
    std::vector<int*> p(10000);
    for(int i = 0 ; i < 10000; i++)
    {
        int* a = static_cast<int*>(allocator.alloc());
        LOG(INFO) << a;
        p[i] = a;
    }

     for(int i = 0 ; i < 10000; i++)
    {
        allocator.free(p[i]);
    } 

    int bbb;
    std::cin >> bbb;
}

#if 0
#include <iostream>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <fstream>
#include <string>
#endif

void printCacheSize() {
#if defined(_WIN32)
    // Windows平台使用Windows API查询缓存信息
    DWORD bufferSize = 0;
    GetLogicalProcessorInformation(nullptr, &bufferSize);
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)malloc(bufferSize);
    if (!buffer) {
        std::cerr << "内存分配失败\n";
        return;
    }
    if (!GetLogicalProcessorInformation(buffer, &bufferSize)) {
        std::cerr << "获取处理器信息失败\n";
        free(buffer);
        return;
    }

    size_t count = bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    for (size_t i = 0; i < count; ++i) {
        if (buffer[i].Relationship == RelationCache) {
            CACHE_DESCRIPTOR cache = buffer[i].Cache;
            std::string level = "L" + std::to_string(cache.Level);
            std::string type;
            switch (cache.Type) {
                case CacheUnified: type = "Unified"; break;
                case CacheInstruction: type = "Instruction"; break;
                case CacheData: type = "Data"; break;
                default: type = "Unknown"; break;
            }
            std::cout << level << " Cache: " << cache.Size / 1024 << " KB, Type: " << type << std::endl;
        }
    }
    free(buffer);

#elif defined(__linux__)
    // Linux平台读取/sys/devices/system/cpu/cpu0/cache/index*/size
    for (int i = 0; i < 4; ++i) {
        std::string path = "/sys/devices/system/cpu/cpu0/cache/index" + std::to_string(i) + "/size";
        std::ifstream file(path);
        if (file) {
            std::string sizeStr;
            std::getline(file, sizeStr);
            std::cout << "Cache index" << i << " size: " << sizeStr << std::endl;
        }
    }
#else
    std::cout << "当前平台暂不支持自动检测缓存大小\n";
#endif
}

TEST_F(TestCommon, testCache)
{
    printCacheSize();
}
#endif