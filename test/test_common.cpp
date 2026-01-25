#include "test_common.h"

#include "Common/DynamicArray.hpp"
#include "Common/HashMap.hpp"
#include "Common/HashSet.hpp"
#include "Common/List.hpp"
#include "Common/Log.hpp"
#include "Common/Matrix.hpp"
#include "Common/MemAllocator.hpp"
#include "Common/Object.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Singleton.hpp"
#include "Common/String.hpp"

TEST_F(TestCommon, testSingle)
{
    class ClassA : public EgLab::Singleton<ClassA>
    {
    public:
        void setA(int _a)
        {
            a = _a;
        }
        int getA()
        {
            return a;
        }

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
    for (int i = 0; i < cppstr.size(); i++)
    {
        EXPECT_EQ(cppstr.c_str()[i], str.c_str()[i]);
    }

    EgLab::String a("a");
    EgLab::String b("b");
    auto c = a + b;
    EXPECT_EQ(c.c_str()[0], 'a');
    EXPECT_EQ(c.c_str()[1], 'b');
}

#include <any>
#include <chrono>
#include <vector>

TEST_F(TestCommon, anyTest)
{
    return; // just for test
    using namespace std;
    using namespace std::chrono;
    const int N = 10000000; // 测试次数

    // 准�?�数�?
    std::vector<int> data(N);
    for (int i = 0; i < N; ++i)
    {
        data[i] = i;
    }

    // 测试 std::any �?�?
    vector<any> anys(N);
    for (int i = 0; i < N; ++i)
    {
        anys[i] = data[i];
    }

    volatile int sum_any = 0; // 防�??优化
    auto start_any = high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
    {
        sum_any += any_cast<int>(anys[i]);
    }
    auto end_any = high_resolution_clock::now();
    auto duration_any = duration_cast<milliseconds>(end_any - start_any).count();

    // 测试 void* �?�?
    vector<void*> void_ptrs(N);
    for (int i = 0; i < N; ++i)
    {
        void_ptrs[i] = &data[i];
    }

    volatile int sum_void = 0; // 防�??优化
    auto start_void = high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
    {
        sum_void += *(static_cast<int*>(void_ptrs[i]));
    }
    auto end_void = high_resolution_clock::now();
    auto duration_void = duration_cast<milliseconds>(end_void - start_void).count();

    cout << "std::any cost: " << duration_any << " ms, sum = " << sum_any << endl;
    cout << "void* cost: " << duration_void << " ms, sum = " << sum_void << endl;
}

TEST_F(TestCommon, MemAllocateTest)
{
    // return; // the case cannot pass beacuse of the complier changed
    EgLab::StaticSizeAllocator<int> allocator;
    int* b = static_cast<int*>(allocator.alloc());
    size_t testNum = 100000;
    std::vector<int*> p(testNum);
    for (int i = 0; i < testNum; i++)
    {
        int* a = static_cast<int*>(allocator.alloc());
        p[i] = a;
    }

    for (int i = 0; i < testNum; i++)
    {
        allocator.free(p[i]);
    }
    allocator.free(b);
}

TEST_F(TestCommon, DynamicArrayTest)
{
    EgLab::DynamicArray<int> arr;
    EXPECT_EQ(arr.size(), 0);
    EXPECT_EQ(arr.capacity(), 0);
    EXPECT_TRUE(arr.empty());

    int a = 1;
    arr.pushBack(a);
    EXPECT_EQ(arr.size(), 1);
    EXPECT_EQ(arr.capacity(), 2); // 默�?�初始�?�量�?0，�?�加�?2

    int b = 2;
    arr.pushBack(b);
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr.capacity(), 2); // 容量不变

    int c = 3;
    arr.pushBack(c);
    EXPECT_EQ(arr.size(), 3);
    EXPECT_GT(arr.capacity(), 2); // 容量应�?��?�加

    EXPECT_EQ(arr[2], 3);

    for (int i = 0; i < arr.size(); i++)
    {
        LOG(INFO) << arr[i];
    }

    class AAA
    {
    public:
        int x;
        int y;
        int z;
    };

    EgLab::DynamicArray<AAA> arr2;
    arr2.resize(1000);

    for (int i = 0; i < arr2.size(); i++)
    {
        arr2[i].x = i;
        arr2[i].y = i * 2;
        arr2[i].z = i * 3;
    }

    for (int i = 0; i < arr2.size(); i++)
    {
        LOG(INFO) << "index :" << i << " x : " << arr2[i].x << " y : " << arr2[i].y
                  << " z : " << arr2[i].z;
    }
}

TEST_F(TestCommon, MatrixTest)
{
    EgLab::Matrix<double, 4, 4> m4(3, 4, 1, 2, 3, 4, 5, 4, 7, 1, 5, 4, 1, 6, 5.25, 76);
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            LOG(INFO) << m4[i][j];
        }
    }
}

TEST_F(TestCommon, ListTest)
{
    EgLab::List<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);
    list.pushBack(4);

    list.popBack();
    list.popBack();

    EgLab::List<int>::Node* current = list.getHead();
    while (current != nullptr)
    {
        LOG(INFO) << current->data;
        current = current->next;
    }
}

TEST_F(TestCommon, ListItTest)
{
    EgLab::List<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);
    list.pushBack(4);

    auto it = list.begin();
    int i = 1;
    while (it.hasNext())
    {
        int& value = it.next();
        EXPECT_EQ(value, i);
        i++;
    }

    i = 1;

    for (auto it = list.begin(); it.hasNext(); it.next())
    {
        int& value = it.data();
        EXPECT_EQ(value, i);
        i++;
    }
}

TEST_F(TestCommon, HashSet)
{
    EgLab::HashSet<int> HashSet;
    int data[] = {1,       3,    2,    1,   4,     5,       6,     6,   7,     1,     12,
                  2,       34,   5314, 5,   13246, 1346,    1346,  134, 6134,  513,   45123,
                  423,     412,  312,  412, 534,   61,      326,   134, 51345, 12233, 2431,
                  3,       12,   312,  41,  25,    134,     5,     413, 5613,  56,    132,
                  6,       1346, 1,    235, 123,   5,       12356, 143, 6,     1342,  76,
                  2548769, 758,  9,    79,  5768,  3657,    6245,  6,   423,   5321,  4,
                  21,      312,  4,    125, 34,    7626243, 5,     13};

    for (int i = 0; i < 73; i++)
    {
        HashSet.insert(data[i]);
    }

    int i = 0;
    for (auto it = HashSet.begin(); it != HashSet.end(); ++it)
    {
        LOG(INFO) << i << " " << *it;
        i++;
    }

    // EXPECT_TRUE(HashSet.insert(1));
    // EXPECT_TRUE(HashSet.insert(2));
    // EXPECT_FALSE(HashSet.insert(1)); // 重复插入
    // EXPECT_EQ(HashSet.size(), 2);
}

TEST_F(TestCommon, HashMap)
{
    EgLab::HashMap<int, int> map;
    map.insert({1, 1});
    map.insert({1, 2});
    map.insert({1, 4});
    map.insert({2, 1});
    for (auto it : map)
    {
        LOG(INFO) << it.first << " " << it.second;
    }
    map.erase(2);
    map.erase(3);
    for (auto it : map)
    {
        LOG(INFO) << it.first << " " << it.second;
    }
}

TEST_F(TestCommon, mix)
{
    int data[] = {1,       3,    2,    1,   4,     5,       6,     6,   7,     1,     12,
                  2,       34,   5314, 5,   13246, 1346,    1346,  134, 6134,  513,   45123,
                  423,     412,  312,  412, 534,   61,      326,   134, 51345, 12233, 2431,
                  3,       12,   312,  41,  25,    134,     5,     413, 5613,  56,    132,
                  6,       1346, 1,    235, 123,   5,       12356, 143, 6,     1342,  76,
                  2548769, 758,  9,    79,  5768,  3657,    6245,  6,   423,   5321,  4,
                  21,      312,  4,    125, 34,    7626243, 5,     13};

    EgLab::DynamicArray<EgLab::List<int>> test;
    for (int i = 0; i < 73; i++)
    {
        EgLab::List<int> l;
        test.resize(i + 1);
        l.pushBack(data[i]);
        test.pushBack(move(l));
    }
}

TEST_F(TestCommon, sharedPtr)
{
    class A
    {
    public:
        A(int v) : _v(v)
        {
            LOG(INFO) << "A()" << " : " << _v;
        }

        A()
        {
            LOG(INFO) << "A()" << " : ";
        }

        ~A()
        {
            LOG(INFO) << "~A()" << " : " << _v;
        }

    private:
        int _v;
    };

    EgLab::SharedPtr<A> a(1);
    EgLab::SharedPtr<A> e = EgLab::makeShared<A>(2);
    EgLab::SharedPtr<A> b(a);
    EgLab::SharedPtr<A> c(2);
    EgLab::SharedPtr<A> d(3);
    a = nullptr;
    b = nullptr;
    c = d;
    c = b;

    LOG(INFO) << "end";
}

TEST_F(TestCommon, uniquePtr)
{
    class A
    {
    public:
        A(int v) : _v(v)
        {
            LOG(INFO) << "A()" << " : " << _v;
        }

        A()
        {
            LOG(INFO) << "A()" << " : ";
        }

        ~A()
        {
            LOG(INFO) << "~A()" << " : " << _v;
        }

    private:
        int _v;
    };
    EgLab::UniquePtr<A> cc;
    cc = EgLab::makeUnique<A>(5);
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
    // Windows平台使用Windows API查�?�缓存信�?
    DWORD bufferSize = 0;
    GetLogicalProcessorInformation(nullptr, &bufferSize);
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)malloc(bufferSize);
    if (!buffer) {
        std::cerr << "内存分配失败\n";
        return;
    }
    if (!GetLogicalProcessorInformation(buffer, &bufferSize)) {
        std::cerr << "获取处理器信�?失败\n";
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
    std::cout << "当前平台暂不�?持自动�?�测缓存大小\n";
#endif
}

TEST_F(TestCommon, testCache)
{
    printCacheSize();
}
#endif